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


#include"../Http/HttpConn.h"
#include"Epoller.h"


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
    int m_port;
    bool m_openLinger;
    int m_timeoutMs;
    bool m_isClose;
    int m_listenFd;
    char* m_srcDir;

    uint32_t m_listenEvent;
    uint32_t m_connEvent;

    void InitEventMode(int triMode);
    bool InitSocket();




};







#endif