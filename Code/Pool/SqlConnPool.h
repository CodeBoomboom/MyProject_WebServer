#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include "../Log/Log.h"

//数据库连接池
class SqlConnPool {
public:
    static SqlConnPool *Instance();//单例

    MYSQL *GetConn();//获取一个Mysql连接
    void FreeConn(MYSQL * conn);//释放一个Mysql连接（实际是又放回到连接池中）
    int GetFreeConnCount();//获取空闲用户数量

    //初始化一个连接
    void Init(const char* host, int port,       //主机名 端口号
              const char* user,const char* pwd, //用户名 密码
              const char* dbName, int connSize);//数据库名 连接数量（初始化为10，表示同一个用户最多10个连接）
    void ClosePool();//关闭数据库连接池

private:
    SqlConnPool();
    ~SqlConnPool();

    int MAX_CONN_;  // 最大的连接数
    int useCount_;  // 当前的用户数
    int freeCount_; // 空闲的用户数

    std::queue<MYSQL *> connQue_;   // 队列（MYSQL *）
    std::mutex mtx_;    // 互斥锁
    sem_t semId_;   // 信号量
};


#endif // SQLCONNPOOL_H