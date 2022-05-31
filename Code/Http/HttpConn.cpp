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