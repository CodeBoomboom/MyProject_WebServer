/********************************************************************
@FileName:Buffer.h
@Version: 1.0
@Notes:   自定义可增长的缓冲区类
@Author:  XiaoDexin
@Email:   xiaodexin0701@163.com
@Date:    2022/05/30 21:18:07
********************************************************************/
#ifndef _BUFFER_H_
#define _BUFFER_H_

#include<cstring>
#include<vector>
#include<atomic>
//#include<iostream>
//#include<unistd.h>
//#include<sys/uio.h>
class Buffer
{
private:

    std::vector<char> m_buffer;
    std::atomic<std::size_t> m_readPos;// 读的位置
    std::atomic<std::size_t> m_writePos;// 写的位置
    
    
public:
    Buffer(int initBufferSize = 1024);
    ~Buffer() = default;

    void RetrieveAll();
};












#endif