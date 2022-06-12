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

//若m_code是40x则将m_path置为对应的路径
//参数:无
//返回值:无
void HttpResponse::ErrorHtml() {
    if(CODE_PATH.count(m_code) == 1) {
        m_path = CODE_PATH.find(m_code)->second;
        stat((m_srcDir + m_path).data(), &m_mmFileStat);
    }
}






