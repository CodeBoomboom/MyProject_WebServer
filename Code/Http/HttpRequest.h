/********************************************************************
@FileName:HttpRestquest.h
@Version: 1.0
@Notes:   客户端请求类，解析客户端请求
@Author:  XiaoDexin
@Email:   xiaodexin0701@163.com
@Date:    2022/05/30 21:13:38
********************************************************************/
#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <errno.h>     
#include <mysql/mysql.h>  //mysql

#include "../Buffer/Buffer.h"
#include "../Log/Log.h"
#include "../Pool/SqlConnPool.h"
#include "../Pool/SqlConnRAII.h"

class HttpRequest {
public:
    enum PARSE_STATE {
        REQUEST_LINE,   // 正在解析请求首行
        HEADERS,        // 头
        BODY,           // 体
        FINISH,         // 完成
    };

    enum HTTP_CODE {
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };
    
    HttpRequest() { Init(); }
    ~HttpRequest() = default;

    void Init();
    bool parse(Buffer& buff);//解析Buffer中的数据

    std::string path() const;//获取路径
    std::string& path();
    std::string method() const;//获取方法
    std::string version() const;//版本
    std::string GetPost(const std::string& key) const;//表单数据
    std::string GetPost(const char* key) const;

    bool IsKeepAlive() const;//是否保持连接

private:
    bool ParseRequestLine_(const std::string& line);//解析请求首行
    void ParseHeader_(const std::string& line);//解析请求头
    void ParseBody_(const std::string& line);//解析请求体

    void ParsePath_();//解析请求路径
    void ParsePost_();//解析Post请求
    void ParseFromUrlencoded_();//解析post请求表单数据

    static bool UserVerify(const std::string& name, const std::string& pwd, bool isLogin);//验证用户

    PARSE_STATE state_;     // 解析的状态
    std::string method_, path_, version_, body_;    // 请求方法，请求路径，协议版本，请求体
    std::unordered_map<std::string, std::string> header_;   // 请求头（键值对）
    std::unordered_map<std::string, std::string> post_;     // post请求表单数据

    static const std::unordered_set<std::string> DEFAULT_HTML;  // 默认的网页
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG; 
    static int ConverHex(char ch);  // 将十六进制字符转换成十进制整数
};


#endif //HTTP_REQUEST_H