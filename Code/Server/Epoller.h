#ifndef _EPOLLER_H_
#define _EPOLLER_H_
/********************************************************************
@FileName:epoller.h
@Version: 1.0
@Notes:   epoll相关类
@Author:  XiaoDexin
@Email:   xiaodexin0701@163.com
@Date:    2022/05/30 19:33:50
********************************************************************/
#include<sys/epoll.h>
#include<fcntl.h>
#include<unistd.h>
#include<vector>
#include<errno.h>

class Epoller{
public:
    explicit Epoller(int maxEvent = 1024);
    
    ~Epoller();

    bool AddFd(int fd, uint32_t events);

    bool ModFd(int fd, uint32_t events);

    bool DelFd(int fd);

    int Wait(int timeoutMs = -1);

    int GetEventFd(size_t i) const;//1）const的函数不能对其数据成员进行修改操作。2）const的对象，不能引用非const的成员函数。

    uint32_t GetEvents(size_t i) const;


private:
    int m_epollFd;    //epoll_create()创建一个epoll对象，返回值就是epollFd

    std::vector<struct epoll_event> m_events;// 检测到的事件的集合 

};








#endif