#include"Buffer.h"

Buffer::Buffer(int initBufferSize):m_buffer(initBufferSize), m_readPos(0), m_writePos(0){}

//缓冲区开始
//参数:无
//返回值:无
char* Buffer::BeginPtr(){
    return &*m_buffer.begin();
}

const char* Buffer::BeginPtr() const {
    return &*m_buffer.begin();
}

//扩容
//参数:无
//返回值:无
void Buffer::MakeSpace(size_t len){
    if(WritableBytes() + PrependadleBytes() < len) {//(前面已经读过的+后面没写的) < len
        m_buffer.resize(m_writePos + len + 1);//扩容
    }
    else{
        //(前面已经读过的+后面没写的) > len
        //把当前vector中已经有的数据（可以读的数据即writePos_ - readPos_）移到最前面
        //这样后面空出来的就是>len的     
        size_t readable = ReadableBytes();//writePos_ - readPos_
        std::copy(BeginPtr() + m_readPos, BeginPtr() + m_writePos, BeginPtr());//把BeginPtr() + readPos到BeginPtr() + writePos之间的数据拷贝到BeginPtr()
        m_readPos = 0;//更新readPos_位置
        m_writePos = m_readPos + readable;//更新writePos_位置
    }

}

// 可以写的数据大小，缓冲区的总大小 - 写位置
size_t Buffer::WritableBytes() const {
    return m_buffer.size() - m_writePos;
}

//可以读的数据大小  写位置-读位置，中间的数据就是可以读的大小
//参数:无
//返回值:无
size_t Buffer::ReadableBytes() const{
    return m_writePos - m_readPos;
}

//前面可用的空间（前面的数据读完了空间就又可以写了），当前读取到哪个位置，就是前面可以用的空间大小
//参数:无
//返回值:无
size_t Buffer::PrependadleBytes() const{
    return m_readPos;
}

//返回的是指向当前vector可读的起始元素的迭代器（可以认为是指向可读元素的指针）
//参数:无
//返回值:当前vector可读的起始元素的迭代器
const char* Buffer::Peek() const{

    return BeginPtr() - m_readPos;
}

//readpos移动len位
//参数:无
//返回值:无
void Buffer::Retrieve(size_t len){
    m_readPos += len;
}

//把readPos_后移到指定位置end上   buff.RetrieveUntil(lineEnd + 2);
//参数:无
//返回值:无
void Buffer::RetrieveUntil(const char* end){
    Retrieve(end - Peek());
}

//初始化(清空)缓冲区
//参数:无
//返回值:无
void Buffer::RetrieveAll() {
    bzero(&m_buffer[0], m_buffer.size());
    m_readPos = 0;
    m_writePos = 0;
}

//将缓冲区中数据存到string中并清空缓冲区，返回string就是缓冲区数据
//参数:无
//返回值:缓冲区数据
std::string Buffer::RetrieveAllToStr() {
    std::string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
}

//当前vector可写的起始元素的迭代器（可以认为是指向可写元素的指针）
//参数:无
//返回值:无
const char* Buffer::BeginWriteConst() const{
    return BeginPtr() + m_writePos;
}

char* Buffer::BeginWrite() {
    return BeginPtr() + m_writePos;
}

//更新writePos,新写了多长的数据writePos_就后移多少位
//参数:无
//返回值:无
void Buffer::HasWritten(size_t len){
    m_writePos += len;
}

void Buffer::Append(const std::string& str) {
    Append(str.data(), str.length());
}

void Buffer::Append(const void* data, size_t len) {
    Append(static_cast<const char*>(data), len);
}

//Append(buff, len - writable);   buff临时数组，len-writable是临时数组中的数据个数
//参数:无
//返回值:无
void Buffer::Append(const char* str, size_t len) {
    EnsureWriteable(len);
    std::copy(str, str + len, BeginWrite());//把buff临时数组中的数据拷贝到扩容后的vector中
    HasWritten(len);//更新writePos_位置
}

//确保可写，若不可写则扩容
//参数:无
//返回值:无
void Buffer::EnsureWriteable(size_t len){
    if(WritableBytes() < len){
        MakeSpace(len);//扩容，扩容长度实际是len - writable，即buff临时数组中的数据个数
    }
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
    else if(static_cast<size_t>(len) <= writeable){//类型强转，ssize_t -> size_t
        m_writePos += len;  
    }
    else{//如果数据大小大于缓冲区大小，即buff中有数据，则需要将m_buffer扩容，然后将buff临时数组中的数据拷贝到m_buffer中
        m_writePos = m_buffer.size();
        Append(buff, len - writeable);
    }
    return len;
}

//向fd中写数据
//参数:fd,传出参数Errno
//返回值:写的字节数
ssize_t Buffer::WriteFd(int fd, int* saveErrno){
    //向fd中写（发送）数据，可以认为是从writeBuff读出来数据后再写（发送）到fd中

    size_t readSize = ReadableBytes();
    ssize_t len = write(fd, Peek(), readSize);
    if(len < 0) {
        *saveErrno = errno;
        return len;
    } 
    m_readPos += len;
    return len;
}