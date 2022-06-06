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
#include<iostream>
#include<unistd.h>
#include<sys/uio.h>
class Buffer
{
private:

    std::vector<char> m_buffer;
    std::atomic<std::size_t> m_readPos;// 读的位置
    std::atomic<std::size_t> m_writePos;// 写的位置
    
    char* BeginPtr();   //获取内存起始位置
    const char* BeginPtr() const;//获取内存起始位置，重载
    void MakeSpace(size_t len);//创建空间
    
public:
    Buffer(int initBufferSize = 1024);
    ~Buffer() = default;

    void RetrieveAll();//清空缓冲区
    size_t WritableBytes() const;//可以写的大小
    size_t ReadableBytes() const;//可以读的大小
    size_t PrependadleBytes() const;//前面可以用的空间
    const char* Peek() const;//可以读的起始元素
    void Retrieve(size_t len);//readPos_前进（后移）len位
    void RetrieveUntil(const char* end);//把readPos_后移到指定位置end上
    std::string RetrieveAllToStr();//将缓冲区中数据存到string中并清空缓冲区，返回string就是缓冲区数据
    const char* BeginWriteConst() const;//可以写的起始元素
    char* BeginWrite();//可以写的起始元素
    void HasWritten(size_t len);//更新写位置指针
    void Append(const std::string &str);
    void Append(const void* data, size_t len);
    void Append(const char* str, size_t len);//当写的长度大于可写长度时，调用此函数，此函数的其他重载都是为了适配不同的数据类型
    void Append(const Buffer& buff);
    void EnsureWriteable(size_t len);//确保可写

    ssize_t ReadFd(int fd, int* Errno);
    ssize_t WriteFd(int fd, int* Errno);
};












#endif