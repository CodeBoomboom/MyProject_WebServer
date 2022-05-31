#include"Buffer.h"

Buffer::Buffer(int initBufferSize):m_buffer(initBufferSize), m_readPos(0), m_writePos(0){}



//初始化缓冲区
//参数:无
//返回值:无
void Buffer::RetrieveAll() {
    bzero(&m_buffer[0], m_buffer.size());
    m_readPos = 0;
    m_writePos = 0;
}



// 可以写的数据大小，缓冲区的总大小 - 写位置
size_t Buffer::WritableBytes() const {
    return m_buffer.size() - m_writePos;
}



//读取fd中的数据，存储在m_buffer中
//参数:fd,传出参数Errno
//返回值:读到的字节数
ssize_t Buffer::ReadFd(int fd, int* Errno){

    //从fd中读数据，可以认为是先从fd中读到数据，然后写到readBuff_中
    char buff[65535];//临时数组

    //分散读，若iov[0]读满了会再读到iov[1]中（读到这两块内存里，其实就是把接收到的数据写到这两块内存里）
    struct iovec iov[2];
    const size_t writeable = WritableBytes();

    /* 分散读， 保证数据全部读完 （读到这两块内存里，其实就是把接收到的数据写到这两块内存里）*/
    iov[0].iov_base = BeginPtr() + m_writePos;
    iov[0].iov_len = writeable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    const ssize_t len = readv(fd, iov, 2);
    if(len < 0) *Errno = errno;
    else if(static_cast<size_t>(len) <= writeable)  m_writePos += len;
    else{
        m_writePos = m_buffer.size();
        Append(buff, len - writeable);

    }
    return len;
}