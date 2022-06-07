#include"Webserver.h"

Webserver::Webserver(
        int port, int triMode, int timeoutMS, bool OptLinger,
        int sqlport, const char* sqlUser, const char* sqlPwd,
        const char* dbName, int connPoolNum, int threadNum,
        bool openLog, int logLevel, int logQueSize):
        m_port(port), m_openLinger(OptLinger), m_timeoutMs(timeoutMS), m_isClose(false),
        m_epoller(new Epoller())
{
    m_srcDir = getcwd(nullptr, 256);// 获取当前的工作路径,当前目录是/home/xiaodexin/桌面/MyProject_WebServer，也就是说只能bin/server来执行

    strncat(m_srcDir, "/Resources",16);// 拼接资源路径,/home/xiaodexin/桌面/MyProject_WebServer/Resources

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
    std::cout<<"create server success"<<std::endl;
}

Webserver::~Webserver()
{
    close(m_listenFd);
    m_isClose = true;
    free(m_srcDir);

}


//服务器启动函数
//参数:  None
//返回值:None
void Webserver::Start(){

    int timeMS = -1;  /* epoll wait timeout == -1 无事件将阻塞 */

    if(!m_isClose){
        std::cout<<"start"<<std::endl;
    }

    while(!m_isClose){

        // timeMS是最先要超时的连接的超时的时间，传递到epoll_wait()函数中
        // 当timeMS时间内有事件发生，epoll_wait()返回，否则等到了timeMS时间后才返回
        // 这样做的目的是为了让epoll_wait()调用次数变少，提高效率
        int eventCnt = m_epoller->Wait(timeMS);//返回产生事件的文件描述符个数

        //循环处理每一个事件
        for(int i = 0; i < eventCnt; i++){
            //处理事件
            int fd = m_epoller->GetEventFd(i);//获取事件对应的fd
            uint32_t events = m_epoller->GetEvents(i);//获取事件类型

            //监听文件描述符有事件发生，说明有新的连接连进来
            if(fd == m_listenFd){
                DealListen();
            }
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))//错误的一些情况
            {
                std::cout<<"events error"<<std::endl;
                CloseClient(&m_users[fd]);
            }
            else if(events & EPOLLIN)    //有数据到达，需要读
            {
                std::cout<<"客户端有数据到达，要读，客户端fd:"<<fd<<std::endl;
                DealRead(&m_users[fd]);//处理读操作
            }





        }


    }
    


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

    //优雅关闭相关
    ret = setsockopt(m_listenFd, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
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
    ret = m_epoller->AddFd(m_listenFd,  m_listenFd | EPOLLIN);
    if(ret == 0) {
        //LOG_ERROR("Add listen error!");
        std::cout<<"add listen error"<<std::endl;
        close(m_listenFd);
        return false;
    }
    SetFdNonblock(m_listenFd);
    std::cout<<"Server port:"<<m_port<<std::endl;
    //LOG_INFO("Server port:%d", m_port);
    return true;
}

//设置文件描述符非阻塞
//参数:文件描述符
//返回值:成功返回文件描述符失败-1
int Webserver::SetFdNonblock(int fd)
{
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}


//m_ListenFd上有事件发生，即有新客户端连接，处理新连接进来的客户端，将其挂到m_epollfd
//参数:无
//返回值:无
void Webserver::DealListen()
{
    struct sockaddr_in  addr;//保存新连接金来的客户端信息
    socklen_t len = sizeof(addr);

    // 如果监听文件描述符设置的是 ET模式，则需要循环把所有连接处理了
    do{
        int fd = accept(m_listenFd, (struct sockaddr *)&addr, &len);
        if(fd < 0) {return;}
        else if(HttpConn::m_userCount >= MAX_FD){
            std::cout<<"HttpConn::m_userCount >= MAX_FD"<<std::endl;
            return;
        }
        AddClient(fd, addr);//添加客户端
    } while (m_listenFd & EPOLLET);
}

//当客户端上有数据时处理读操作，读取数据
//参数:客户端HttpConn对象
//返回值:无
void Webserver::DealRead(HttpConn* client){
    //ExtentTime_(client);   // 延长这个客户端的超时时间
    // 加入到队列中等待线程池中的线程处理（读取数据）
    std::cout<<"加入到队列中等待线程池中的线程处理（读取数据）"<<std::endl;
    m_threadpool->AddTask(std::bind(&Webserver::OnRead, this, client));//std::bind:位于function文件中，是一个函数适配器，接受一个可调用对象。参1：函数名，参2：右值引用？，参3：函数参数
    
}

//添加客户端
//参数:fd：添加的客户端fd，addr：fd要监听的事件
//返回值:无
void Webserver::AddClient(int fd, sockaddr_in addr)
{
    std::cout<<"new client socketfd:"<<fd<<std::endl;
    //添加HttpConn对象
    m_users[fd].Init(fd,addr);//初始化为HttpConn对象，键为文件描述符，值为HttpConn对象

    //添加定时器
    // if(timeoutMS_ > 0) {//timeoutMS_定时器超时时间
    //     // 添加到定时器对象中，当检测到超时时执行CloseConn_函数进行关闭连接
    //     timer_->add(fd, timeoutMS_, std::bind(&WebServer::CloseConn_, this, &users_[fd]));
    // }

    //挂到m_epoller上
    m_epoller->AddFd(fd, EPOLLIN | m_connEvent);
    SetFdNonblock(fd);
    std::cout<<"已将该客户端加到epoll上"<<std::endl;
}


void Webserver::CloseClient(HttpConn* client)
{
    //LOG_INFO("Client[%d] quit!", client->GetFd());
    m_epoller->DelFd(client->GetFd());//从epoll树上摘下
    client->Close();
}

//子线程处理函数，读取客户端数据
//这个方法是在子线程中执行的（读取数据），Reactor模式
//参数:客户端
//返回值:无
void Webserver::OnRead(HttpConn* client){
    int ret = -1;
    int readErrno = 0;
    std::cout<<"read data..."<<std::endl;
    ret = client->read(&readErrno);//读客户端数据
    if(ret <= 0 && readErrno != EAGAIN){
        std::cout<<"read error"<<std::endl;
        CloseClient(client);
        return;
    }
    // 业务逻辑的处理（解析请求+生成响应）
    OnProcess(client);
}



void Webserver::OnProcess(HttpConn* client)
{

}

