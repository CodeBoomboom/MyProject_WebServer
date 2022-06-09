#ifndef _SQLCONNRAII_H_
#define _SQLCONNRAII_H_

#include "SqlConnPool.h"
/* 资源在对象构造初始化 资源在对象析构时释放*/
class SqlConnRAII{
public:
    SqlConnRAII(MYSQL** sql, SqlConnPool *connpool)//为啥用二级指针？很简单，为了最终能修改connpool->GetConn()所指向的连接池对象,,可以看最下面的示例
    {
        assert(connpool);
        *sql = connpool->GetConn();//*sql:取值，取二级指针sql的值，其值其实是一级指针的地址，也就是一个一级指针。此处是把二级指针sql的值赋为connpool->GetConn()所代表的一级指针。
                                  //若用一级指针的话，无法修改connpool->GetConn()所指向的连接对象     
        m_sql = *sql;
        m_connpool = connpool;
    }



private:

    MYSQL *m_sql;
    SqlConnPool* m_connpool;




};


// void test()
// {
//     int b = 10;
//     int *a = &b;
//     change(&a);
//     printf("%d\n, a");
// }

// void change(int** a){
//     int c = 10;
//     *a = &c;//这样才能修改b的值
// }






#endif