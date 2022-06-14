#include "HeapTimer.h"

//插入一个新节点（定时器），并调整其到合适位置（小根堆位置），从下向上调整
void HeapTimer::siftup_(size_t i) {
    assert(i >= 0 && i < heap_.size());
    size_t j = (i - 1) / 2;//父节点索引
    while(j >= 0) {
        if(heap_[j] < heap_[i]) { break; }//比较父节点与新加入的节点大小，若父节点小就break，否则交换
        SwapNode_(i, j);
        i = j;
        j = (i - 1) / 2;//一直向上找父节点，直到根节点
    }
}

//交换两个节点值
void HeapTimer::SwapNode_(size_t i, size_t j) {
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;//更新小根堆中文件描述符fd和其在小根堆中索引的关系
    ref_[heap_[j].id] = j;
} 

//调整指定索引的节点（定时器）到合适位置，从该索引处向下调整（向下依次比较其与其子节点的值的大小）
//返回1：调整完毕
//返回0：未进行调整（该索引处的节点值还是大于其子节点）
bool HeapTimer::siftdown_(size_t index, size_t n) {
    assert(index >= 0 && index < heap_.size());
    assert(n >= 0 && n <= heap_.size());
    size_t i = index;
    size_t j = i * 2 + 1;//左子节点
    while(j < n) {
        if(j + 1 < n && heap_[j + 1] < heap_[j]) j++;//根节点（指定索引节点）大于其左子节点，则j+1，比较其与其右子节点的大小
        if(heap_[i] < heap_[j]) break;//根节点（指定索引节点）大于其子节点，直接break，不用调整位置
        SwapNode_(i, j);//否则，交换根节点（指定索引节点）与其子节点
        i = j;
        j = i * 2 + 1;//继续向下调整
    }
    return i > index;
}

//向小根堆中添加一个节点
//id：文件描述符
//timeout：超时时间
//cb：超时回调函数（可调用对象）
void HeapTimer::add(int id, int timeout, const TimeoutCallBack& cb) {
    assert(id >= 0);
    size_t i;
    if(ref_.count(id) == 0) {//该文件描述符在小根堆中没有
        /* 新节点：堆尾插入，调整堆 */
        i = heap_.size();//获取小根堆大小
        ref_[id] = i;//添加文件描述符和索引关系
        heap_.push_back({id, Clock::now() + MS(timeout), cb});//添加小根堆节点（其实就是id客户端定时器）到小根堆最后，超时时间为当前时间+timeout
        siftup_(i); // 向上调整，跟父亲比较
    } 
    else {
        /* 已有结点：调整堆 */
        i = ref_[id];//获取其在小根堆的索引
        heap_[i].expires = Clock::now() + MS(timeout);//更新其超时时间
        heap_[i].cb = cb;//更新超时回调函数
        if(!siftdown_(i, heap_.size())) {//先向下调整，看更新的超时时间是否大于其子节点的超时时间了没，若大了就向下调整
            siftup_(i);//否则向上调整（其实不太可能到这一步，因为超时时间更新的话一般都是延长）
        }
    }
}

//删除指定id结点，并触发回调函数
//没用到
void HeapTimer::doWork(int id) {
    /* 删除指定id结点，并触发回调函数 */
    if(heap_.empty() || ref_.count(id) == 0) {
        return;
    }
    size_t i = ref_[id];
    TimerNode node = heap_[i];
    node.cb();
    del_(i);
}

//删除小根堆中指定节点
void HeapTimer::del_(size_t index) {
    /* 删除指定位置的结点 */
    assert(!heap_.empty() && index >= 0 && index < heap_.size());
    /* 将要删除的结点换到队尾，然后调整堆 */
    size_t i = index;
    size_t n = heap_.size() - 1;
    assert(i <= n);
    if(i < n) {
        SwapNode_(i, n);//交换该索引节点与小根堆最后一个节点
        if(!siftdown_(i, n)) {//将交换后的小根堆进行向上向下调整
            siftup_(i);
        }
    }
    /* 队尾元素删除 */
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

//调整指定id的结点
void HeapTimer::adjust(int id, int timeout) {
    /* 调整指定id的结点 */
    assert(!heap_.empty() && ref_.count(id) > 0);
    heap_[ref_[id]].expires = Clock::now() + MS(timeout);;//修改超时时间
    siftdown_(ref_[id], heap_.size());//延时时间只会加大，所以只向下调整就行
}

void HeapTimer::tick() {
    /* 清除超时结点 */
    if(heap_.empty()) {
        return;
    }
    while(!heap_.empty()) {
        TimerNode node = heap_.front();//小根堆的根节点
        if(std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) { //超时时间-当前时间若>0，说明还未超时，直接break
            break; 
        }
        node.cb();//否则就是超时了,执行超时回调
        pop();
    }
}

//pop出超时时间最小的节点，其实就是根节点
void HeapTimer::pop() {
    assert(!heap_.empty());
    del_(0);
}

//清空小根堆
void HeapTimer::clear() {
    ref_.clear();
    heap_.clear();
}

//获取下一个将要超时的节点剩余的时间
int HeapTimer::GetNextTick() {
    tick();//清除已经超时的
    size_t res = -1;
    if(!heap_.empty()) {
        res = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now()).count();//最小的超时时间-当前时间
        if(res < 0) { res = 0; }
    }
    return res;
}