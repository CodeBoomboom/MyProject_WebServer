/********************************************************************
@FileName:ThreadPool.h
@Version: 1.0
@Notes:   线程池类
@Author:  XiaoDexin
@Email:   xiaodexin0701@163.com
@Date:    2022/05/31 20:12:51
********************************************************************/
#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>//条件变量
#include <assert.h>
#include <queue>
#include <thread>//C++11的线程库，底层也会用到linux的线程库
#include <functional>//回调相关
class ThreadPool {
public:
    //explicit:防止构造函数隐式转换
    explicit ThreadPool(size_t threadCount = 8): pool_(std::make_shared<Pool>()) {//std::make_shared<Pool>是在动态内存中分配一个Pool对象并初始化它，返回指向此对象的shared_ptr智能指针
            assert(threadCount > 0);//断言：测试用的

            // 创建threadCount个子线程
            for(size_t i = 0; i < threadCount; i++) {
                std::thread([pool = pool_] {    //线程要执行的代码
                    std::unique_lock<std::mutex> locker(pool->mtx);//创建一个互斥锁locker，初始化为pool->mtx
                    while(true) {
                        if(!pool->tasks.empty()) {
                            // 从任务队列中取第一个任务
                            auto task = std::move(pool->tasks.front());//move：将左值转换为右值引用
                            // 移除掉队列中第一个元素
                            pool->tasks.pop();
                            locker.unlock();
                            task();//任务执行的代码,,   function:调用task中的对象，参数为空
                            locker.lock();
                        } 
                        else if(pool->isClosed) break;
                        else pool->cond.wait(locker);   // 如果队列为空，等待，条件变量只能用unique_lock类型的锁作为参数，而不能用lock_guard
                    }
                }).detach();// 线程分离
            }
    }

    ThreadPool() = default;//无参构造函数采用默认实现

    ThreadPool(ThreadPool&&) = default;//右值引用
    
    ~ThreadPool() {
        if(static_cast<bool>(pool_)) {
            {
                /*
                lock_guard简化了 lock/unlock 的写法,
                 lock_guard在构造时自动锁定互斥量, 而在退出作用域时会析构自动解锁, 保证了上锁解锁的正确操作, 正是典型的 RAII 机制
                */
                std::lock_guard<std::mutex> locker(pool_->mtx);//创建一个互斥锁locker，初始化为pool->mtx
                pool_->isClosed = true;
            }
            pool_->cond.notify_all();//把所有线程都唤醒，break
        }
    }

    //向任务队列中添加一个任务
    template<class F>
    void AddTask(F&& task) {
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);//创建一个互斥锁locker，初始化为pool->mtx
            pool_->tasks.emplace(std::forward<F>(task));//在线程池中的任务队列的尾部添加一个任务，std::forward：实现完美转发
        }
        pool_->cond.notify_one();   // 唤醒一个等待的线程
    }

private:
    // 线程池结构体
    struct Pool {
        std::mutex mtx;     // 互斥锁
        std::condition_variable cond;   // 条件变量
        bool isClosed;          // 是否关闭
        std::queue<std::function<void()>> tasks;    // 队列（保存的是任务），创建了一个queue队列，里面存储的是调用形式为void()类型（返回值为void，参数为空）的可调用对象（lambda匿名函数、函数等，此处可以认为就是一个函数）
    };
    std::shared_ptr<Pool> pool_;  //  池子,shared_ptr:智能指针
};


#endif //THREADPOOL_H