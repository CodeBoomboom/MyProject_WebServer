/********************************************************************
@FileName:Webserver.h
@Version: 1.0
@Notes:   服务器类
@Author:  XiaoDexin
@Email:   xiaodexin0701@163.com
@Date:    2022/05/29 14:17:25
********************************************************************/
#ifndef _WEBSERVER_H_
#define _WEBSERVER_H_

#include<unordered_map>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
#include<assert.h>
#include<errno.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<iostream>
#include<memory>//unique_ptr

#include"../Http/HttpConn.h"
#include"Epoller.h"
#include"../Pool/ThreadPool.h"


class Webserver{
public:
    Webserver(
        int port, int tirMode, int timeoutMS, bool OptLinger,   /* 端口 ET模式 timeoutMs 优雅退出  */                                                       
        int sqlport, const char* sqlUser, const char* sqlPwd,   /* Mysql配置  Mysql端口号 用户名 用户密码 */
        const char* dbName, int connPoolNum, int threadNum,     /* 数据库名称 数据库连接池数量 线程池数量*/
        bool openLog, int logLevel, int logQueSize              /* 日志开关 日志等级 日志异步队列容量 */
    );
    ~Webserver();
    void Start();

private:
    static const int MAX_FD = 65536;    // 最大的文件描述符的个数

    int m_port;//端口
    bool m_openLinger;//优雅关闭
    int m_timeoutMs;//延时时间
    bool m_isClose;//服务器关闭
    int m_listenFd;//监听描述符
    char* m_srcDir;//资源目录

    uint32_t m_listenEvent;//监听描述符事件
    uint32_t m_connEvent;//连接客户端描述符事件

    std::unique_ptr<ThreadPool> m_threadpool;//线程池
    std::unique_ptr<Epoller> m_epoller;//epoll对象
    std::unordered_map<int, HttpConn> m_users;//保存的是客户端连接的信息，键为文件描述符，值为HttpConn对象

    void InitEventMode(int triMode);
    bool InitSocket();
    void AddClient(int fd, sockaddr_in addr);
    void CloseClient(HttpConn* client);

    int SetFdNonblock(int fd);// 设置文件描述符非阻塞

    void DealListen();
    void DealRead(HttpConn* cilent);
    void DealWrite(HttpConn* client);

    void OnRead(HttpConn* client);






};







#endif