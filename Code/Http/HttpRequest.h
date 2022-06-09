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
#include <assert.h>

#include "../Buffer/Buffer.h"
#include "../Pool/SqlConnPool.h"
#include "../Pool/SqlConnRAII.h"
class HttpRequest
{
public:
    enum PARSE_STATE {
        REQUEST_LINE,   // 正在解析请求首行
        HEADERS,        // 头
        BODY,           // 体
        FINISH,         // 完成
    };


    HttpRequest(){ Init();}
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
    bool ParseRequestLine(const std::string& line);//解析请求首行
    void ParseHeader(const std:: string& line);//解析请求头
    void ParseBody(const std::string& line);//解析请求体

    void ParsePath();//解析请求路径
    void ParsePost();//解析Post请求
    void ParseFromUrlencoded();//解析post表单数据

    static bool UserVerify(const std::string& name, const std::string& pwd, bool isLogin);//验证用户

    PARSE_STATE m_state;// 解析的状态
    std::string m_method, m_path, m_version, m_body;// 请求方法，请求路径，协议版本，请求体
    std::unordered_map<std::string , std::string> m_header;//请求头（键值对）
    std::unordered_map<std::string, std::string> m_post;     // post请求表单数据(用户名+密码)

    static const std::unordered_set<std::string> DEFAULT_HTML;// 默认的网页
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
    static int ConverHex(char ch);  // 将十六进制字符转换成十进制整数
};




#endif