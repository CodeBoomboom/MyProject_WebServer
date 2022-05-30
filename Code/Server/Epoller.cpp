#include "Epoller.h"

//创建epoll对象
//参数:最大事件数
//返回值:无
Epoller::Epoller(int maxEvent):m_epollFd(epoll_create(512)), m_events(maxEvent){} 

Epoller::~Epoller()
{
    close(m_epollFd);
}

//向epoll中添加一个fd
//参数:fd， fd的监听事件
//返回值:成功true失败false
bool Epoller::AddFd(int fd, uint32_t events)
{
    if(fd < 0)  return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(m_epollFd, EPOLL_CTL_ADD, fd, &ev);
}

//修改epoll中文件描述符的监听事件
//参数:fd，修改的事件
//返回值:成功true失败false
bool Epoller::ModFd(int fd, uint32_t events)
{
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(m_epollFd, EPOLL_CTL_MOD, fd, &ev);   
}

// 删除
bool Epoller::DelFd(int fd) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    return 0 == epoll_ctl(m_epollFd, EPOLL_CTL_DEL, fd, &ev);
}

//调用epoll_wait进行事件检测
//参数:延时时间
//返回值:成功返回有多少描述符就绪，时间到时返回0，出错-1
int Epoller::Wait(int timeoutMs)
{
    return epoll_wait(m_epollFd, &m_events[0], static_cast<int>(m_events.size()), timeoutMs);
}

//获取产生事件的文件描述符
//参数:size_t i :第i个产生事件的文件描述符
//返回值:该文件描述符的fd
int Epoller::GetEventFd(size_t i) const {
    return m_events[i].data.fd;
}

//获取文件描述符产生的事件
//参数:size_t i；第i个产生事件的文件描述符所产生的事件
//返回值:无
uint32_t Epoller::GetEvents(size_t i) const {
    return m_events[i].events;
}