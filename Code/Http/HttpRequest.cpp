#include"HttpRequest.h"
using namespace std;

const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML{
            "/index", "/register", "/login",
             "/welcome", "/video", "/picture", };

const std::unordered_map<std::string, int> HttpRequest::DEFAULT_HTML_TAG {
            {"/register.html", 0}, {"/login.html", 1},  };


//初始化请求对象信息
//参数:无
//返回值:无
void HttpRequest::Init()
{
    m_method = m_body = m_path = m_version = "";
    m_state = REQUEST_LINE;
    m_header.clear();
    m_post.clear();
}

bool HttpRequest::IsKeepAlive() const{
    if(m_header.count("Connection") == 1){
        return m_header.find("Connection")->second == "keep-alive" && m_version == "1.1";
    }
    return false;
}

//解析请求数据
//参数:无
//返回值:无
bool HttpRequest::parse(Buffer& buff)
{
    const char CRLF[] = "\r\n"; //行结束符
    if(buff.ReadableBytes() <= 0){
        return false;
    }

    //buff中有数据可读，且状态没有到FINSH，就一直解析
    while(buff.ReadableBytes() && m_state != FINISH)
    {
        //获取一行数据,根据/r/n为结束标志
        const char* lineEnd = std::search(buff.Peek(), buff.BeginWriteConst(), CRLF, CRLF + 2);//在buff.Peek()到buff.BeginWriteConst()间寻找首次出现/r/n的地方，返回指针
        std::string line(buff.Peek(), lineEnd);
        switch(m_state)
        {
            case REQUEST_LINE:
                // 解析请求首行
                if(!ParseRequestLine(line)){
                    return false;
                }
                ParsePath();//解析出请求资源路径
                break;
            case HEADERS:
                // 解析请求头
                ParseHeader(line);
                if(buff.ReadableBytes() <= 2){
                    m_state = FINISH;
                }
                break;
            case BODY:
                ParseBody(line);
                break;
            default:
                break;
        }
        if(lineEnd == buff.BeginWrite()) { break; }//当lineEnd = writePos_时说明解析完，直接break
        buff.RetrieveUntil(lineEnd + 2);//每解析完一行readPos_都要移到lineEnd + 2位置
    }
    //LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

//解析路径
//参数:无
//返回值:无
void HttpRequest::ParsePath() {
    // 如果访问根目录，默认表示访问index.html
    // 例如 http://192.168.110.111:10000/
    if(m_path == "/") {
        m_path = "/index.html"; 
    }
    else {
        // 其他默认的一些页面
        // 例如 http://192.168.110.111:10000/regist
        for(auto &item: DEFAULT_HTML) {
            if(item == m_path) {
                m_path += ".html";
                break;
            }
        }
    }
}

//解析请求首行
//参数:无
//返回值:无
bool HttpRequest::ParseRequestLine(const std::string& line) {
    // GET / HTTP/1.1
    std::regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");//正则表达式匹配，括号代表不同组的数据
    std::smatch subMatch;//匹配到的数据会保存在这里面，每组数据依次放在subMatch[1]、subMatch[2]...
    if(regex_match(line, subMatch, patten)) {   //regex_match:匹配函数
        m_method = subMatch[1];
        m_path = subMatch[2];
        m_version = subMatch[3];
        m_state = HEADERS;
        return true;
    }
    //LOG_ERROR("RequestLine Error");
    return false;
}

//解析请求头
//参数:无
//返回值:无
void HttpRequest::ParseHeader(const std::string& line) {
    std::regex patten("^([^:]*): ?(.*)$");
    std::smatch subMatch;
    if(regex_match(line, subMatch, patten)) {
        m_header[subMatch[1]] = subMatch[2];
    }
    else {
        m_state = BODY;
    }
}

//解析请求体
//参数:无
//返回值:无
void HttpRequest::ParseBody(const std::string& line) {
    m_body = line;
    ParsePost();//只有post请求有请求体
    m_state = FINISH;
    //LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
}


//将十六进制的字符，转换成十进制的整数  用于加密
//参数:无
//返回值:无
int HttpRequest::ConverHex(char ch) {
    if(ch >= 'A' && ch <= 'F') return ch -'A' + 10;
    if(ch >= 'a' && ch <= 'f') return ch -'a' + 10;
    return ch;
}

//解析Post请求
//参数:无
//返回值:无
void HttpRequest::ParsePost() {
    if(m_method == "POST" && m_header["Content-Type"] == "application/x-www-form-urlencoded") {//表单提交
        // 解析表单信息
        ParseFromUrlencoded();//解析到m_post
        if(DEFAULT_HTML_TAG.count(m_path)) {
            int tag = DEFAULT_HTML_TAG.find(m_path)->second;
            //LOG_DEBUG("Tag:%d", tag);
            if(tag == 0 || tag == 1) {
                bool isLogin = (tag == 1);
                if(UserVerify(m_post["username"], m_post["password"], isLogin)) {
                    m_path = "/welcome.html";
                } 
                else {
                    m_path = "/error.html";
                }
            }
        }
    }   
}


//解析表单信息
//username=hello&password=hello
void HttpRequest::ParseFromUrlencoded() {
    if(m_body.size() == 0) { return; }
    // username=zhangsan&password=123
    std::string key, value;//键值对
    int num = 0;
    int n = m_body.size();
    int i = 0, j = 0;

    for(; i < n; i++) {
        char ch = m_body[i];
        switch (ch) {
        case '=':
            key = m_body.substr(j, i - j);//username      =hello&password=hello  把user拿出来赋给key
            j = i + 1;
            break;
        case '+':
            m_body[i] = ' ';
            break;
        case '%':           
            //中文注册时，简单的加密的操作(中文用户名)，编码
            //使注册的中文编码与在mysql中保存的编码不一样，这样别人就不会通过mysql数据反编出用户名和密码了
            num = ConverHex(m_body[i + 1]) * 16 + ConverHex(m_body[i + 2]);
            m_body[i + 2] = num % 10 + '0';
            m_body[i + 1] = num / 10 + '0';
            i += 2;
            break;
        case '&':
            value = m_body.substr(j, i - j);//username=     hello    &password=hello
            j = i + 1;
            m_post[key] = value;//键值对加入post_表单
            //LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
            break;
        default:
            break;
        }
    }
    //assert(j <= i);
    if(m_post.count(key) == 0 && j < i) {
        value = m_body.substr(j, i - j);
        m_post[key] = value;
    }
}

//验证用户信息（注册/登录）
//参数:无
//返回值:无
// 用户验证（整合了登录和注册的验证）
bool HttpRequest::UserVerify(const std::string &name, const std::string &pwd, bool isLogin) {
    if(name == "" || pwd == "") { return false; }
    //LOG_INFO("Verify name:%s pwd:%s", name.c_str(), pwd.c_str());
    MYSQL* sql;
    SqlConnRAII(&sql,  SqlConnPool::Instance());
    assert(sql);
    
    bool flag = false;
    unsigned int j = 0;
    char order[256] = { 0 };
    MYSQL_FIELD *fields = nullptr;
    MYSQL_RES *res = nullptr;
    
    if(!isLogin) { flag = true; }
    /* 查询用户及密码 */
    snprintf(order, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1", name.c_str());//拼接一个select语句放在order里
    //LOG_DEBUG("%s", order);

    if(mysql_query(sql, order)) { //mysql_query：mysql执行函数，成功返回0  数据库查找
        mysql_free_result(res);//查询失败，释放结果
        return false; 
    }

    res = mysql_store_result(sql);//保存查询结果
    j = mysql_num_fields(res);//获取字段
    fields = mysql_fetch_fields(res);//提取字段

    while(MYSQL_ROW row = mysql_fetch_row(res)) {//提取一行数据
        //LOG_DEBUG("MYSQL ROW: %s %s", row[0], row[1]);
        string password(row[1]);//转换为字符串
        /* 登录行为 判断密码是否一致*/
        if(isLogin) {
            if(pwd == password) { flag = true; }
            else {
                flag = false;
                //LOG_DEBUG("pwd error!");
            }
        } 
        else { 
            flag = false; 
            //LOG_DEBUG("user used!");
        }
    }
    mysql_free_result(res);

    /* 注册行为 且 用户名未被使用*/
    if(!isLogin && flag == true) {
        //LOG_DEBUG("regirster!");
        bzero(order, 256);
        snprintf(order, 256,"INSERT INTO user(username, password) VALUES('%s','%s')", name.c_str(), pwd.c_str());
        //LOG_DEBUG( "%s", order);
        if(mysql_query(sql, order)) { 
            // LOG_DEBUG( "Insert error!");
            flag = false; 
        }
        flag = true;
    }
    SqlConnPool::Instance()->FreeConn(sql); //注册/登录验证完关闭连接，重新放回数据库连接池
    //LOG_DEBUG( "UserVerify success!!");
    return flag;
}

std::string HttpRequest::path() const{
    return m_path;
}

std::string& HttpRequest::path(){
    return m_path;
}

std::string HttpRequest::method() const {
    return m_method;
}

std::string HttpRequest::version() const {
    return m_version;
}

std::string HttpRequest::GetPost(const std::string& key) const {
    assert(key != "");
    if(m_post.count(key) == 1) {
        return m_post.find(key)->second;
    }
    return "";
}

std::string HttpRequest::GetPost(const char* key) const {
    assert(key != nullptr);
    if(m_post.count(key) == 1) {
        return m_post.find(key)->second;
    }
    return "";
}
