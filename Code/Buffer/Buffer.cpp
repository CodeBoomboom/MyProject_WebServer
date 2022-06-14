 #include "Buffer.h"

Buffer::Buffer(int initBuffSize) : buffer_(initBuffSize), readPos_(0), writePos_(0) {}

// 可以读的数据的大小  写位置 - 读位置，中间的数据就是可以读的大小
size_t Buffer::ReadableBytes() const {  
    return writePos_ - readPos_;
}

// 可以写的数据大小，缓冲区的总大小 - 写位置
size_t Buffer::WritableBytes() const {
    return buffer_.size() - writePos_;
}

// 前面可以用的空间，当前读取到哪个位置，就是前面可以用的空间大小
size_t Buffer::PrependableBytes() const {
    return readPos_;
}

//返回的是指向当前vector可读的起始元素的迭代器（可以认为是指向可读元素的指针）
const char* Buffer::Peek() const {
    return BeginPtr_() + readPos_;
}

//readPos_前进（后移）len位
void Buffer::Retrieve(size_t len) {
    assert(len <= ReadableBytes());
    readPos_ += len;
}

//buff.RetrieveUntil(lineEnd + 2);
//把readPos_后移到指定位置end上
void Buffer::RetrieveUntil(const char* end) {
    assert(Peek() <= end );
    Retrieve(end - Peek());
}

void Buffer::RetrieveAll() {
    bzero(&buffer_[0], buffer_.size());
    readPos_ = 0;
    writePos_ = 0;
}

std::string Buffer::RetrieveAllToStr() {
    std::string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
}

//返回的是当前vector可写的起始元素的迭代器（可以认为是指向可写元素的指针）
const char* Buffer::BeginWriteConst() const {
    return BeginPtr_() + writePos_;
}

char* Buffer::BeginWrite() {
    return BeginPtr_() + writePos_;
}

//更新writePos_
//新写了多长的数据writePos_就后移多少位
void Buffer::HasWritten(size_t len) {
    writePos_ += len;
} 

void Buffer::Append(const std::string& str) {
    Append(str.data(), str.length());
}

void Buffer::Append(const void* data, size_t len) {
    assert(data);
    Append(static_cast<const char*>(data), len);
}

//  Append(buff, len - writable);   buff临时数组，len-writable是临时数组中的数据个数
void Buffer::Append(const char* str, size_t len) {
    assert(str);
    EnsureWriteable(len);
    std::copy(str, str + len, BeginWrite());//把buff临时数组中的数据拷贝到扩容后的vector中
    HasWritten(len);//更新writePos_位置
}

void Buffer::Append(const Buffer& buff) {
    Append(buff.Peek(), buff.ReadableBytes());
}

void Buffer::EnsureWriteable(size_t len) {
    if(WritableBytes() < len) {
        MakeSpace_(len);    //扩容，扩容长度实际是len - writable，即buff临时数组中的数据个数
    }
    assert(WritableBytes() >= len);
}

ssize_t Buffer::ReadFd(int fd, int* saveErrno) {
    
    //从fd中读数据，可以认为是先从fd中读到数据，然后写到readBuff_中

    char buff[65535];   // 临时的数组，保证能够把所有的数据都读出来
    
    //分散读，若iov[0]读满了会再读到iov[1]中（读到这两块内存里，其实就是把接收到的数据写到这两块内存里）
    struct iovec iov[2];
    const size_t writable = WritableBytes();
    
    /* 分散读， 保证数据全部读完 （读到这两块内存里，其实就是把接收到的数据写到这两块内存里）*/
    iov[0].iov_base = BeginPtr_() + writePos_;
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    const ssize_t len = readv(fd, iov, 2);
    if(len < 0) {
        *saveErrno = errno;
    }
    else if(static_cast<size_t>(len) <= writable) { //类型强转，ssize_t -> size_t
        writePos_ += len;
    }
    else {
        writePos_ = buffer_.size();
        Append(buff, len - writable);//如果数据大小大于缓冲区大小，即buff中有数据，则需要将buffer_扩容，然后将buff临时数组中的数据拷贝到buffer_中
    }
    return len;
}

ssize_t Buffer::WriteFd(int fd, int* saveErrno) {

    //向fd中写（发送）数据，可以认为是从writeBuff_读出来数据后再写（发送）到fd中

    size_t readSize = ReadableBytes();
    ssize_t len = write(fd, Peek(), readSize);
    if(len < 0) {
        *saveErrno = errno;
        return len;
    } 
    readPos_ += len;
    return len;
}

//返回指向所创建的vector的第一个元素的迭代器（可以浅显的认为是指针，指向vector数组首地址）
char* Buffer::BeginPtr_() {
    return &*buffer_.begin();
}

const char* Buffer::BeginPtr_() const {
    return &*buffer_.begin();
}

void Buffer::MakeSpace_(size_t len) {
    if(WritableBytes() + PrependableBytes() < len) {//(前面已经读过的+后面没写的) < len
        buffer_.resize(writePos_ + len + 1);
    } 
    else {
        //(前面已经读过的+后面没写的) > len
        //把当前vector中已经有的数据（可以读的数据即writePos_ - readPos_）移到最前面
        //这样后面空出来的就是>len的
        size_t readable = ReadableBytes();//writePos_ - readPos_
        std::copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_());//把BeginPtr_() + readPos_到BeginPtr_() + writePos_之间的数据拷贝到BeginPtr_()
        readPos_ = 0;//更新readPos_位置
        writePos_ = readPos_ + readable;//更新writePos_位置
        assert(readable == ReadableBytes());//移完后的数据大小是不变的
    }
}