/********************************************************************
@FileName:HttpConn.h
@Version: 1.0
@Notes:   客户端连接类
@Author:  XiaoDexin
@Email:   xiaodexin0701@163.com
@Date:    2022/05/29 14:41:07
********************************************************************/
#ifndef _HTTPCONN_H_
#define _HTTPCONN_H_

#include<sys/types.h>
#include<sys/uio.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<errno.h>



class HttpConn{
public:

    static const char* m_srcDir;//资源目录
    static int m_userCount; // 总共的客户端的连接数
    static bool m_isET;//是否是ET模式

private:




};







#endif