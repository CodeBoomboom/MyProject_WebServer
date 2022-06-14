/********************************************************************
@FileName:Buffer.h
@Version: 1.0
@Notes:   None
@Author:  XiaoDexin
@Email:   xiaodexin0701@163.com
@Date:    2022/06/14 20:51:13
********************************************************************/
#ifndef BUFFER_H
#define BUFFER_H
#include <cstring>   //perror
#include <iostream>
#include <unistd.h>  // write
#include <sys/uio.h> //readv
#include <vector> //readv
#include <atomic>
#include <assert.h>
class Buffer {
public:
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;

    size_t WritableBytes() const;//可写的字节数       
    size_t ReadableBytes() const ;//可读的字节数
    size_t PrependableBytes() const;//可以扩展的字节数

    const char* Peek() const;
    void EnsureWriteable(size_t len);//确保可写
    void HasWritten(size_t len);//写进去

    void Retrieve(size_t len);
    void RetrieveUntil(const char* end);

    void RetrieveAll() ;
    std::string RetrieveAllToStr();

    const char* BeginWriteConst() const;
    char* BeginWrite();

    void Append(const std::string& str);
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);
    void Append(const Buffer& buff);

    ssize_t ReadFd(int fd, int* Errno); 
    ssize_t WriteFd(int fd, int* Errno);

private:
    char* BeginPtr_();              // 获取内存起始位置
    const char* BeginPtr_() const;  // 获取内存起始位置（重载）
    void MakeSpace_(size_t len);    // 创建空间

    std::vector<char> buffer_;  // 具体装数据的vector
    std::atomic<std::size_t> readPos_;  // 读的位置
    std::atomic<std::size_t> writePos_; // 写的位置
};

#endif //BUFFER_H