/********************************************************************
@FileName:HttpRestquest.h
@Version: 1.0
@Notes:   客户端请求类，解析客户端请求
@Author:  XiaoDexin
@Email:   xiaodexin0701@163.com
@Date:    2022/05/30 21:13:38
********************************************************************/
#ifndef _HTTPREQUEST_H_
#define _HTTPREQUEST_H_

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <errno.h>     
#include <mysql/mysql.h>  //mysql

#include "../Buffer/Buffer.h"
class HttpRequest
{
private:
    
public:
    HttpRequest();
    ~HttpRequest();

    void Init();
    bool parse(Buffer& buff);//解析Buffer中的数据

    std::string path() const;//获取路径
    std::string& path();
    std::string method() const;//获取方法
    std::string version() const;//版本
    std::string GetPost(const std::string& key) const;//表单数据
    std::string GetPost(const char* key) const;

    bool IsKeepAlive() const;//是否保持连接
};




#endif