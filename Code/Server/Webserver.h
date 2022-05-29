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

class Webserver{
public:


private:
    int m_port;
    bool m_openLinger;
    int m_timeoutMs;
    bool m_isClose;
    int m_listenFd;
    char* m_srcDir;

};







#endif