/********************************************************************
@FileName:BlockQueue.h
@Version: 1,0
@Notes:   None
@Author:  XiaoDexin
@Email:   xiaodexin0701@163.com
@Date:    2022/06/14 21:04:00
********************************************************************/
#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include <sys/time.h>

//自己封装的阻塞队列
//阻塞队列BlockDequ是一个支持两个附加操作的队列。
//这两个附加的操作是：在队列为空时，获取元素的线程会等待队列变为非空。当队列满时，存储元素的线程会等待队列可用
template<class T>
class BlockDeque {
public:
    explicit BlockDeque(size_t MaxCapacity = 1000);//初始化队列最大容量为1000

    ~BlockDeque();

    void clear();//清空

    bool empty();//是否为空

    bool full();//是否满

    void Close();//关闭

    size_t size();//队列大小

    size_t capacity();//队列容量

    T front();//返回队列第一个元素

    T back();//返回队列最后一个元素

    void push_back(const T &item);//从尾部加入一个元素

    void push_front(const T &item);//从头部加入一个元素

    bool pop(T &item);//移除队首元素

    bool pop(T &item, int timeout);//移除队首元素，设置超时时间

    void flush();//刷新

private:
    std::deque<T> deq_;

    size_t capacity_;//容量

    std::mutex mtx_;//互斥锁

    bool isClose_;//是否关闭

    std::condition_variable condConsumer_;//消费者条件变量

    std::condition_variable condProducer_;//生产者条件变量
};


template<class T>
BlockDeque<T>::BlockDeque(size_t MaxCapacity) :capacity_(MaxCapacity) {
    assert(MaxCapacity > 0);
    isClose_ = false;
}

template<class T>
BlockDeque<T>::~BlockDeque() {
    Close();
};

template<class T>
void BlockDeque<T>::Close() {
    {   
        std::lock_guard<std::mutex> locker(mtx_);
        deq_.clear();//清空队列
        isClose_ = true;
    }
    condProducer_.notify_all();//唤醒所有阻塞在条件变量上的线程
    condConsumer_.notify_all();
};

template<class T>
void BlockDeque<T>::flush() {
    condConsumer_.notify_one();//唤醒一个阻塞在消费者条件变量上的线程
};

template<class T>
void BlockDeque<T>::clear() {
    std::lock_guard<std::mutex> locker(mtx_);
    deq_.clear();
}

template<class T>
T BlockDeque<T>::front() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.front();
}

template<class T>
T BlockDeque<T>::back() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.back();
}

template<class T>
size_t BlockDeque<T>::size() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.size();
}

template<class T>
size_t BlockDeque<T>::capacity() {
    std::lock_guard<std::mutex> locker(mtx_);
    return capacity_;
}

template<class T>
void BlockDeque<T>::push_back(const T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq_.size() >= capacity_) {
        condProducer_.wait(locker);//容器满了，阻塞等待生产者变量
    }
    deq_.push_back(item);
    condConsumer_.notify_one();//唤醒阻塞在消费者变量上的一个线程
}

template<class T>
void BlockDeque<T>::push_front(const T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq_.size() >= capacity_) {
        condProducer_.wait(locker);//阻塞等待生产者变量
    }
    deq_.push_front(item);
    condConsumer_.notify_one();//消费者唤醒
}

template<class T>
bool BlockDeque<T>::empty() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.empty();
}

template<class T>
bool BlockDeque<T>::full(){
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.size() >= capacity_;
}

template<class T>
bool BlockDeque<T>::pop(T &item) {//pop出队头元素，存到item中
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq_.empty()){
        condConsumer_.wait(locker);//容器为空，阻塞消费者条件变量
        if(isClose_){
            return false;
        }
    }
    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();//生产者
    return true;
}

template<class T>
bool BlockDeque<T>::pop(T &item, int timeout) {
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq_.empty()){
        if(condConsumer_.wait_for(locker, std::chrono::seconds(timeout)) 
                == std::cv_status::timeout){
            return false;
        }
        if(isClose_){
            return false;
        }
    }
    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();
    return true;
}

#endif // BLOCKQUEUE_H