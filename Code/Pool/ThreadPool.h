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
        std::queue<std::function<void()>> tasks;// 队列（保存的是任务），创建了一个queue队列，里面存储的是调用形式为void()类型（返回值为void，参数为空）的可调用对象（lambda匿名函数、函数等，此处可以认为就是一个函数）
    };
    std::shared_ptr<Pool> m_pool;//线程池
    
public:
    //explicit:防止构造函数隐式转换
    explicit ThreadPool();
    ~ThreadPool();
};

ThreadPool::ThreadPool()
{
}

ThreadPool::~ThreadPool()
{
}










#endif