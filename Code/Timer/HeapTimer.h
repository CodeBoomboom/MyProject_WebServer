/********************************************************************
@FileName:HeapTimer.h
@Version: 1.0
@Notes:   None
@Author:  XiaoDexin
@Email:   xiaodexin0701@163.com
@Date:    2022/06/14 21:12:21
********************************************************************/
#ifndef HEAP_TIMER_H
#define HEAP_TIMER_H

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h> 
#include <functional> 
#include <assert.h> 
#include <chrono>
#include "../Log/Log.h"

typedef std::function<void()> TimeoutCallBack;//超时回调函数
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;//超时时间

struct TimerNode {
    int id;//该节点对应的客户端文件描述符 
    TimeStamp expires;//超时时间
    TimeoutCallBack cb;//超时回调函数
    bool operator<(const TimerNode& t) {
        return expires < t.expires;//重载<运算符，比较当前t的超时时间和要比较的t的超时时间，若小于就返回true
    }
};
class HeapTimer {
public:
    HeapTimer() { heap_.reserve(64); }

    ~HeapTimer() { clear(); }
    
    void adjust(int id, int newExpires);

    void add(int id, int timeOut, const TimeoutCallBack& cb);

    void doWork(int id);

    void clear();

    void tick();

    void pop();

    int GetNextTick();

private:
    void del_(size_t i);//删除i索引的元素
    
    void siftup_(size_t i);//向上调整

    bool siftdown_(size_t index, size_t n);//向下调整

    void SwapNode_(size_t i, size_t j);

    std::vector<TimerNode> heap_;//vector数组实现的

    std::unordered_map<int, size_t> ref_;//保存小根堆中文件描述符fd和其在小根堆中索引的关系的map
};

#endif //HEAP_TIMER_H