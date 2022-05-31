/********************************************************************
@FileName:ThreadPool.h
@Version: 1.0
@Notes:   线程池类
@Author:  XiaoDexin
@Email:   xiaodexin0701@163.com
@Date:    2022/05/31 20:12:51
********************************************************************/
#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_
#include<mutex>
#include<condition_variable>
#include<queue>
#include<thread>
#include<functional>

class ThreadPool
{
private:
    struct Pool{
        std::mutex m_mtx;//互斥锁
        std::condition_variable m_cond;//条件变量
        bool m_isClose;//是否关闭
        std::queue<std::function<void()>> m_tasks;// 队列（保存的是任务），创建了一个queue队列，里面存储的是调用形式为void()类型（返回值为void，参数为空）的可调用对象（lambda匿名函数、函数等，此处可以认为就是一个函数）
    };
    std::shared_ptr<Pool> m_pool;//线程池
    
public:
    //explicit:防止构造函数隐式转换
    //线程池初始线程数：8
    explicit ThreadPool(size_t threadCount = 8):m_pool(std::make_shared<Pool>()){////std::make_shared<Pool>是在动态内存中分配一个Pool对象并初始化它，返回指向此对象的shared_ptr智能指针
    
        //循环创建子线程
        for(size_t i = 0; i< threadCount; i++){
            std::thread([pool = m_pool]{//下面是子线程要执行的代码
                std::unique_lock<std::mutex> locker(pool->m_mtx);//创建一个互斥锁locker，初始化为pool->mtx
                while(true){
                    if(!pool->m_tasks.empty()){
                        // 从任务队列中取第一个任务
                        auto task = std::move(pool->m_tasks.front());//move：将左值转换为右值引用?
                        pool->m_tasks.pop();// 移除掉队列中第一个元素
                        locker.unlock();
                        task();//任务执行的代码,,   function:调用task中的对象，参数为空
                        locker.lock();
                    }
                    else if(pool->m_isClose)    break;
                    else pool->m_cond.wait(locker);// 如果队列为空，等待，条件变量只能用unique_lock类型的锁作为参数，而不能用lock_guard
                }
            }).detach();//线程分离
        }
    }

    ThreadPool(ThreadPool&&) = default;//右值引用
    ThreadPool() = default;//无参构造函数采用默认实现
    ~ThreadPool(){
        if(static_cast<bool>(m_pool)){
            /*
            lock_guard简化了 lock/unlock 的写法,
                lock_guard在构造时自动锁定互斥量, 而在退出作用域时会析构自动解锁, 保证了上锁解锁的正确操作, 正是典型的 RAII 机制
            */
            std::lock_guard<std::mutex> locker(m_pool->m_mtx);//创建一个互斥锁locker，初始化为pool->mtx
            m_pool->m_isClose = true;
        }
        m_pool->m_cond.notify_all();//把所有线程都唤醒，break
    }
    template<class T>
    void AddTask(T&& task){
        {
            std::lock_guard<std::mutex> locker(m_pool->m_mtx);
            m_pool->m_tasks.emplace(std::forward<T>(task));//在线程池中的任务队列的尾部添加一个任务，std::forward：实现完美转发?
        }
        m_pool->m_cond.notify_one();// 唤醒一个等待的线程
    }
};


#endif