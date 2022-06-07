/********************************************************************
@FileName:HttpConn.h
@Version: 1.0
@Notes:   客户端连接类，每个连接的客户端都会有一个此类对象
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
#include<unistd.h>
#include<iostream>

#include"../Buffer/Buffer.h"
#include"HttpRequest.h"
#include"HttpResponse.h"

class HttpConn{
public:
    HttpConn();

    ~HttpConn();

    void Close();

    void Init(int sockFd, const sockaddr_in& addr);

    ssize_t read(int* saveErrno);//将客户端发送的数据读到readBuff_中
    ssize_t write(int* saveErrno);//writeBuff_及文件信息数据分散写回客户端
    bool process();//客户端处理

    int GetFd() const;
    struct sockaddr_in GetAddr() const;
    const char* GetIP() const;
    int GetPort() const;

    int ToWriteBytes(){
        return m_iov[0].iov_len + m_iov[1].iov_len;
    }

    bool IsKeepAlive() const{
        return m_request.IsKeepAlive();
    }

    static const char* m_srcDir;//资源目录
    static int m_userCount; // 总共的客户端的连接数
    static bool m_isET;//是否是ET模式

private:
    int m_fd;//对应的客户端的fd
    struct sockaddr_in m_addr;//对应客户端的地址结构

    bool m_isClose;//客户端是否关闭

    int m_iovCnt;// 分散内存的数量
    struct iovec m_iov[2];//分散内存


    Buffer m_readBuff;//读（请求）缓冲区，保存请求数据的内容
    Buffer m_writeBuff;//写(响应)缓冲区，保存响应数据的内容

    HttpRequest m_request; // 请求对象
    HttpResponse m_response;// 响应对象


};







#endif