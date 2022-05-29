#include"Webserver.h"

Webserver::Webserver(
        int port, int triMode, int timeoutMS, bool OptLinger,
        int sqlport, const char* sqlUser, const char* sqlPwd,
        const char* dbName, int connPoolNum, int threadNum,
        bool openLog, int logLevel, int logQueSize)
{
    m_srcDir = getcwd(nullptr, 256);

    strncat(m_srcDir, "/Resources",16);

    HttpConn::m_userCount = 0;
    HttpConn::m_srcDir = m_srcDir;

    //初始化数据库连接池
    //SqlConnPool::Instance()->Init("loaclhost", sqlport, sqlUser, sqlPwd, dbName, connPoolNum);

    //初始化事件模式
    InitEventMode(triMode);

    //初始化网络通信内容
    if(!InitSocket()){
        m_isClose = true;
    }

    if(openLog){
        //初始化日志相关信息

    }

}


//服务器启动函数
//参数:  None
//返回值:None
void Webserver::Start(){



}

//事件模式初始化函数
//参数:事件模式
//1：客户端fd ET   2:监听描述符fd ET
//3：客户端fd以及监听描述符fd 都为ET
//返回值:None
void Webserver::InitEventMode(int triMode){
    m_listenEvent = EPOLLRDHUP;
    m_connEvent = EPOLLONESHOT | EPOLLRDHUP;
    switch (triMode)
    {
    case 0:
        break;
    case 1:
        m_connEvent |= EPOLLET;
        break;
    case 2:
        m_listenEvent |= EPOLLET;
        break;
    case 3:
        m_connEvent |= EPOLLET;
        m_listenEvent |= EPOLLET;
        break;
    default:
        m_connEvent |= EPOLLET;
        m_listenEvent |= EPOLLET;
        break;
    }
    HttpConn::m_isET = (m_connEvent & EPOLLET);
}

//socket相关初始化
//参数:无
//返回值:成功true失败false
bool Webserver::InitSocket(){
    int ret;
    struct sockaddr_in addr;
    if(m_port > 65535 || m_port < 1024){
        std::cout<<"端口号过大或过小！"<<std::endl;
        return false;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    struct linger optLinger = { 0 };
    if(m_openLinger) {
        /* 优雅关闭: 直到所剩数据发送完毕或超时 */
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }

    m_listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(m_listenFd < 0){
        std::cout<<"listenfd error"<<std::endl;
        return false;
    }

    ret = setsockopt(m_listenFd, SOL_SOCKET, SO_LINGER, &m_openLinger, sizeof(m_openLinger));
    if(ret < 0) {
        close(m_listenFd);
        std::cout<<"set linger error"<<std::endl;
        //LOG_ERROR("Init linger error!", port_);
        return false;
    }

    int optval = 1;
    /* 端口复用 */
    /* 只有最后一个套接字会正常接收数据。 */
    ret = setsockopt(m_listenFd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret == -1) {
        //LOG_ERROR("set socket setsockopt error !");
        std::cout<<"set resuseaddr error"<<std::endl;
        close(m_listenFd);
        return false;
    }

    ret = bind(m_listenFd, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        //LOG_ERROR("Bind Port:%d error!", port_);
        std::cout<<"bind error"<<std::endl;
        close(m_listenFd);
        return false;
    }

    ret = listen(m_listenFd, 6);
    if(ret < 0) {
        //LOG_ERROR("Listen port:%d error!", port_);
        std::cout<<"listen error"<<std::endl;
        close(m_listenFd);
        return false;
    }

    //添加到epoll树上
    ret = epoller_->AddFd(m_listenFd,  m_listenFd | EPOLLIN);
    if(ret == 0) {
        //LOG_ERROR("Add listen error!");
        close(m_listenFd);
        return false;
    }
    SetFdNonblock(m_listenFd);
    std::cout<<"Server port:"<<m_port<<std::endl;
    //LOG_INFO("Server port:%d", m_port);
    return true;
}












