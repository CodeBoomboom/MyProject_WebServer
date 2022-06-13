#include"HttpResponse.h"

using namespace std;

// 文件后缀 对应的 MIME-TYPE类型，用在响应头Content-Type中
const unordered_map<string, string> HttpResponse::SUFFIX_TYPE = {
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "},
};

// 响应状态码对应的描述语
const unordered_map<int, string> HttpResponse::CODE_STATUS = {
    { 200, "OK" },
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
};

// 响应码对应的资源路径
const unordered_map<int, string> HttpResponse::CODE_PATH = {
    { 400, "/400.html" },
    { 403, "/403.html" },
    { 404, "/404.html" },
};

HttpResponse::HttpResponse()
{
    m_code = -1;
    m_path = m_srcDir = "";
    m_isKeepAlive = false;
    m_mmFile = nullptr; 
    m_mmFileStat = { 0 };
}

HttpResponse::~HttpResponse()
{
    UnmapFile();
}

void HttpResponse::Init(const string& srcDir, string& path, bool isKeepAlive, int code)
{
    assert(srcDir != "");

    if(m_mmFile){ UnmapFile(); }//若内存映射指针不为空则要先释放

    m_code = code;
    m_isKeepAlive = isKeepAlive;
    m_path = path;
    m_srcDir = srcDir;
    m_mmFile = nullptr;
    m_mmFileStat = { 0 };
}

//生成响应信息
//参数:无
//返回值:无
void HttpResponse::MakeResponse(Buffer& buff) {
    /* 判断请求的资源文件 */
    // index.html
    // /home/nowcoder/WebServer-master/resources/index.html
    if(stat((m_srcDir + m_path).data(), &m_mmFileStat) < 0 || S_ISDIR(m_mmFileStat.st_mode)) {
        m_code = 404;
    }
    else if(!(m_mmFileStat.st_mode & S_IROTH)) {
        //无权限
        m_code = 403;
    }
    else if(m_code == -1) { 
        m_code = 200; 
    }
    ErrorHtml();//若code_为40X则将path_置为对应的路径
    AddStateLine(buff);//依次往buff(实际就是writeBuff_)中添加响应行、头、体
    AddHeader(buff);
    AddContent(buff);
}

char* HttpResponse::File() {
    return m_mmFile;
}

size_t HttpResponse::FileLen() const {
    return m_mmFileStat.st_size;
}

//若m_code是40x则将m_path置为对应的路径
//参数:无
//返回值:无
void HttpResponse::ErrorHtml() {
    if(CODE_PATH.count(m_code) == 1) {
        m_path = CODE_PATH.find(m_code)->second;
        stat((m_srcDir + m_path).data(), &m_mmFileStat);
    }
}

// 添加响应状态行
void HttpResponse::AddStateLine(Buffer& buff) {
    string status;
    if(CODE_STATUS.count(m_code) == 1) {
        status = CODE_STATUS.find(m_code)->second;
    }
    else {
        m_code = 400;
        status = CODE_STATUS.find(400)->second;
    }
    buff.Append("HTTP/1.1 " + to_string(m_code) + " " + status + "\r\n");
}

// 添加响应头
void HttpResponse::AddHeader(Buffer& buff) {
    buff.Append("Connection: ");
    if(m_isKeepAlive) {
        buff.Append("keep-alive\r\n");
        buff.Append("keep-alive: max=6, timeout=120\r\n");
    } else{
        buff.Append("close\r\n");
    }
    buff.Append("Content-type: " + GetFileType() + "\r\n");
}

// 添加响应体
//这个函数实际只向writeBuff_中添加了Content-length:
//实际的文件内容是mmap到了mmFile_
//最终是在httpconn中用分散写把writeBuff_和mmFile_依次写给客户端
void HttpResponse::AddContent(Buffer& buff) {
    int srcFd = open((m_srcDir + m_path).data(), O_RDONLY);
    if(srcFd < 0) { 
        ErrorContent(buff, "File NotFound!");
        return; 
    }

    /* 将文件映射到内存提高文件的访问速度 
        MAP_PRIVATE 建立一个写入时拷贝的私有映射*/
    //LOG_DEBUG("file path %s", (srcDir_ + path_).data());
    int* mmRet = (int*)mmap(0, m_mmFileStat.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
    if(*mmRet == -1) {
        ErrorContent(buff, "File NotFound!");
        return; 
    }
    m_mmFile = (char*)mmRet;
    close(srcFd);
    buff.Append("Content-length: " + to_string(m_mmFileStat.st_size) + "\r\n\r\n");
}

string HttpResponse::GetFileType() {
    /* 判断文件类型 */
    string::size_type idx = m_path.find_last_of('.');//find_last_of：从后往前找匹配的字符，如果path_中不存在'.'，则返回string::npos，成功返回位置
    if(idx == string::npos) {
        return "text/plain";//纯文本
    }
    string suffix = m_path.substr(idx);//返回以path_中以idx下标为起始的string
    if(SUFFIX_TYPE.count(suffix) == 1) {
        return SUFFIX_TYPE.find(suffix)->second;
    }
    return "text/plain";
}

// 解除内存映射
void HttpResponse::UnmapFile() {
    if(m_mmFile) {
        munmap(m_mmFile, m_mmFileStat.st_size);
        m_mmFile = nullptr;
    }
}


//这个函数其实就是用html语言写了一个未找到文件的错误界面
void HttpResponse::ErrorContent(Buffer& buff, string message) 
{
    string body;
    string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if(CODE_STATUS.count(m_code) == 1) {
        status = CODE_STATUS.find(m_code)->second;
    } else {
        status = "Bad Request";
    }
    body += to_string(m_code) + " : " + status  + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";

    buff.Append("Content-length: " + to_string(body.size()) + "\r\n\r\n");
    buff.Append(body);
}