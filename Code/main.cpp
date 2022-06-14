#include <unistd.h>
#include "Server/WebServer.h"

int main() {
    /* 守护进程 后台运行 */
    // daemon(1, 0); 

    WebServer MyWebServer(
        9871, 3, 60000, false,             /* 端口 ET模式 timeoutMs 优雅退出  */
        3306, "root", "root", "webserver", /* Mysql配置  Mysql端口号 用户名 用户密码 数据库名称*/
        12, 6, true, 1, 1024);             /* 数据库连接池数量 线程池数量 日志开关 日志等级 日志异步队列容量 */
    
    
    // 启动服务器
    MyWebServer.Start();
} 
  