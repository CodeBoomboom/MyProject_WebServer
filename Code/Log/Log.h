/********************************************************************
@FileName:Log.h
@Version: 1.0
@Notes:   None
@Author:  XiaoDexin
@Email:   xiaodexin0701@163.com
@Date:    2022/06/14 21:03:36
********************************************************************/
#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <string>
#include <thread>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>           // vastart va_end
#include <assert.h>
#include <sys/stat.h>         //mkdir
#include "BlockQueue.h"
#include "../Buffer/Buffer.h"

class Log {
public:
    void init(int level, const char* path = "./log", 
                const char* suffix =".log",
                int maxQueueCapacity = 1024);

    static Log* Instance();
    static void FlushLogThread();

    void write(int level, const char *format,...);
    void flush();

    int GetLevel();
    void SetLevel(int level);
    bool IsOpen() { return isOpen_; }
    
private:
    Log();
    void AppendLogLevelTitle_(int level);
    virtual ~Log();
    void AsyncWrite_();

private:
    static const int LOG_PATH_LEN = 256;//日志路径长度
    static const int LOG_NAME_LEN = 256;//日志文件名称长度
    static const int MAX_LINES = 50000;//最大行，一个日志文件最多存储5w行文件，超过5w行就新开文件存储

    const char* path_;//日志路径
    const char* suffix_;//生成的日志文件的后缀名

    int MAX_LINES_;//最大行，一个日志文件最多存储5w行文件，超过5w行就新开文件存储

    int lineCount_;//行计数，当前日志文件已经写了多少行
    int toDay_;//日期，当前日期发生变化就重开新文件存储日志

    bool isOpen_;
 
    Buffer buff_;//存储日志信息的Buffer
    int level_;//设置存储日志的级别
    bool isAsync_;//是否异步

    FILE* fp_;//操作日志文件的指针
    std::unique_ptr<BlockDeque<std::string>> deque_; //日志队列，非阻塞队列（内含日志信息）
    std::unique_ptr<std::thread> writeThread_;//写线程
    std::mutex mtx_;//互斥锁
};

//宏函数
//level：传递日志级别，//0——3，数值越大级别越高
//format：传递格式
#define LOG_BASE(level, format, ...) \
    do {\
        Log* log = Log::Instance();\
        if (log->IsOpen() && log->GetLevel() <= level) {\
            log->write(level, format, ##__VA_ARGS__); \
            log->flush();\
        }\
    } while(0);


//##：预处理拼接符（宏拼接符）,用于类似函数的宏的替换部分，还可以用于类似对象的宏的替换部分。##放在宏的替换部分的前面，用于宏展开（即宏替换）后，立即将宏中位于##右边的宏替换部分与该宏中位于##左边的部分相拼接至一个整体。
//如：#define XNAME(n) x##n 
//宏调用 int XNAME(4) = 1；
//宏展开（即宏替换）后，我们得到： int x4 = 1;
//这也就体现出了##对其左右部分（即左x和右4）的拼接作用，最终拼接成x4
//__VA_ARGS__代表宏参数中的“...”   如：#define debug(...) printf(__VA_ARGS__)
//__VA_ARGS__的前面加上##是为了用来支持出现0个可变参数的情况
#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif //LOG_H