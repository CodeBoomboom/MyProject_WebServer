#ifndef _SQLCONNPOOL_H_
#define _SQLCONNPOOL_H_

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include <assert.h>

class SqlConnPool
{
public:
    static SqlConnPool * Instance();//单例

    MYSQL *GetConn();//获取一个Mysql连接
    void FreeConn(MYSQL * conn);//释放一个Mysql连接（实际是又放回到连接池中）
    int GetFreeConnCount();//获取空闲用户数量

    void Init(const char* host, int port,
              const char* user,const char* pwd, 
              const char* dbName, int connSize);//初始化一个连接
    void ClosePool();//关闭数据库连接池

private:
    SqlConnPool();
    ~SqlConnPool();

    int MAX_CONN_;  // 最大的连接数
    int m_useCount;  // 当前的用户数
    int m_freeCount; // 空闲的用户数

    std::queue<MYSQL *> m_connQue;   // 队列（MYSQL *）
    std::mutex m_mtx;    // 互斥锁
    sem_t m_semId;   // 信号量

};









#endif