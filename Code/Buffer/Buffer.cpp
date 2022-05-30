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