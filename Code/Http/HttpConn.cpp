#include"HttpConn.h"

const char* HttpConn::m_srcDir;
int HttpConn::m_userCount;
bool HttpConn::m_isET;

HttpConn::HttpConn() { 
    m_fd = -1;
    m_addr = { 0 };
    m_isClose = true;
};

HttpConn::~HttpConn() { 
    Close(); 
};

void HttpConn::Close()
{
    //response_.UnmapFile();  // 解除内存映射
    if(m_isClose == false){
        m_isClose = true; 
        m_userCount--;
        close(m_fd);
        std::cout<<"client close"<<std::endl;
        //LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
    }        
}

//初始化客户端信息
//参数:客户端fd，客户端addr
//返回值:无
void HttpConn::Init(int sockFd, const sockaddr_in& addr)
{
    m_userCount++;
    m_fd = sockFd;
    m_addr = addr;

    //初始化读写缓冲
    m_readBuff.RetrieveAll();
    m_writeBuff.RetrieveAll();

    m_isClose = false;
    std::cout<<"已创建该客户端的HttpConn对象"<<std::endl;
    //LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
}

int HttpConn::GetFd() const {
    return m_fd;
};

struct sockaddr_in HttpConn::GetAddr() const{
    return m_addr;
}

const char* HttpConn::GetIP() const {
    return inet_ntoa(m_addr.sin_addr);
}

int HttpConn::GetPort() const {
    return m_addr.sin_port;
}

//将客户端发送的数据读到readBuff_中
//参数:错误号
//返回值:无
ssize_t HttpConn::read(int* saveErrno)
{
    //一次性读出所有数据
    ssize_t len = -1;
    do{
        len = m_readBuff.ReadFd(m_fd, saveErrno);
        if(len < 0){
            break;
        }
    }while(m_isET);
    return len;
}

//writeBuff_及文件信息数据分散写回客户端
//参数:无
//返回值:无
ssize_t HttpConn::write(int* saveErrno)
{
    ssize_t len = -1;
    do{
        //分散写数据
        len = writev(m_fd, m_iov, m_iovCnt);
        if(len < 0){
            *saveErrno = errno;
            break;
        }

        // 这种情况是所有数据都传输结束了
        if(m_iov[0].iov_len + m_iov[1].iov_len == 0){ break;}
        // 写到了第二块内存，做相应的处理
        else if(static_cast<size_t>(len) > m_iov[0].iov_len){
            m_iov[1].iov_base = (uint8_t*) m_iov[1].iov_base + (len - m_iov[0].iov_len);
            m_iov[1].iov_len -= (len - m_iov[0].iov_len);
            if(m_iov[0].iov_len){//写完后将m_writeBuff清空
                m_writeBuff.RetrieveAll();
                m_iov[0].iov_len = 0;
            }
        }
        //还没有写到第二块内存的数据
        else{
            m_iov[0].iov_base = (uint8_t*)m_iov[0].iov_base + len;
            m_iov[0].iov_len -= len;
            m_writeBuff.Retrieve(len);//向客户端写相当于从m_writeBuff中读出再写
        }
    }while(m_isET || ToWriteBytes() > 10240);
    return len;
}

//业务逻辑处理（解析数据/生成响应）
bool HttpConn::process()
{
    //初始化请求对象
    m_request.Init();

    if(m_readBuff.ReadableBytes() <= 0)//没请求数据
    {
        return false;
    }
    else if(m_request.parse(m_readBuff)){// 解析请求数据
        //LOG_DEBUG("%s", request_.path().c_str());
        // 解析完请求数据以后，初始化响应对象
        m_response.Init(m_srcDir, m_request.path(), m_request.IsKeepAlive(), 200);
    }
    else{//解析失败
        m_response.Init(m_srcDir, m_request.path(), false, 400);
    }

    // 生成响应信息（writeBuff_中保存着响应的一些信息）
    m_response.MakeResponse(m_writeBuff);
    //响应头
    m_iov[0].iov_base = const_cast<char*>(m_writeBuff.Peek());
    m_iov[0].iov_len = m_writeBuff.ReadableBytes();
    m_iovCnt = 1;    

    //文件
    if(m_response.FileLen() > 0 && m_response.File())
    {
        m_iov[1].iov_base = m_response.File();
        m_iov[1].iov_len = m_response.FileLen();
        m_iovCnt = 2;
    }

    // LOG_DEBUG("filesize:%d, %d  to %d", response_.FileLen() , iovCnt_, ToWriteBytes());
    return true;

}