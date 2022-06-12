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
#include <assert.h>

#include "../Buffer/Buffer.h"
class HttpResponse
{
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
private:
    void AddStateLine_(Buffer &buff);
    void AddHeader_(Buffer &buff);
    void AddContent_(Buffer &buff);

    void ErrorHtml_();
    std::string GetFileType_();




    int m_code;//响应状态码
    bool m_isKeepAlive;  // 是否保持连接

    std::string m_path;  // 资源的路径 /index.html
    std::string m_srcDir;    // 资源的目录 /home/xiaodexin/WebServer_notes/resources
    
    char* m_mmFile;  // 文件内存映射的指针
    struct stat m_mmFileStat;    // 文件的状态信息

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;  // 后缀 - 类型
    static const std::unordered_map<int, std::string> CODE_STATUS;    // 状态码 - 描述 
    static const std::unordered_map<int, std::string> CODE_PATH;      // 状态码 - 路径

    

};











#endif