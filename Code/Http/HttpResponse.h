/********************************************************************
@FileName:HttpResponse.h
@Version: 1.0
@Notes:   客户端响应类，生成客户端响应
@Author:  XiaoDexin
@Email:   xiaodexin0701@163.com
@Date:    2022/05/30 21:15:57
********************************************************************/
#ifndef _HTTPRESPONSE_H_
#define _HTTPRESPONSE_H_
#include"../Buffer/Buffer.h"
#include <unordered_map>
#include <fcntl.h>       // open
#include <unistd.h>      // close
#include <sys/stat.h>    // stat
#include <sys/mman.h>    // mmap, munmap
class HttpResponse
{
private:
    int m_code;//响应状态码
    
public:
    HttpResponse();
    ~HttpResponse();

    void Init(const std::string& srcDir, std::string& path, bool isKeepAlive = false, int code = -1);
    void MakeResponse(Buffer& buff);//生成响应信息
    void UnmapFile();
    char* File();
    size_t FileLen() const;
    void ErrorContent(Buffer& buff, std::string message);
    int Code() const { return m_code; }
};











#endif