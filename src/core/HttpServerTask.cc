#include "workflow/HttpUtil.h"
#include "workflow/HttpMessage.h"

#include <arpa/inet.h>

#include "HttpServerTask.h"
#include "HttpServer.h"
#include "StrUtil.h"

using namespace protocol;

#define HTTP_KEEPALIVE_DEFAULT    (60 * 1000)  // 默认的 HTTP Keep-Alive 超时时间（60秒）
#define HTTP_KEEPALIVE_MAX        (300 * 1000) // 最大的 HTTP Keep-Alive 超时时间（300秒）

namespace Yukino
{
// HttpServerTask 类的构造函数
HttpServerTask::HttpServerTask(CommService *service,
                                ProcFunc& process) :
        WFServerTask(service, WFGlobal::get_scheduler(), process),
        req_is_alive_(false),
        req_has_keep_alive_header_(false)
{
    // 设置任务的回调函数
    WFServerTask::set_callback([this](HttpTask *task) {
        for(auto &cb : cb_list_)
        {
            cb(task);
        }
    });
}

// 处理 HTTP 请求和响应的方法
// 在这里主要分析HTTP请求中是否有Keep-Alive头部，然后调用父类接口
// 如Keep-Alive: timeout=5, max=50
void HttpServerTask::handle(int state, int error)
{
    // 如果任务状态是 WFT_STATE_TOREPLY（需要回复客户端）
    if (state == WFT_STATE_TOREPLY)
    {
        // 检查请求是否支持 Keep-Alive
        req_is_alive_ = this->req.is_keep_alive();
        // 检查请求是否包含 Keep-Alive 头部
        if (req_is_alive_ && this->req.has_keep_alive_header())
        {
            // 创建一个 HTTP 头部游标，用于解析请求头部
            HttpHeaderCursor req_cursor(&this->req);
            struct HttpMessageHeader header{};

            // 设置要查找的头部名称为 "Keep-Alive"
            header.name = "Keep-Alive";
            header.name_len = strlen("Keep-Alive");
            // 在请求头部中查找 "Keep-Alive" 头部
            req_has_keep_alive_header_ = req_cursor.find(&header);
            // 如果找到 "Keep-Alive" 头部，保存其值
            if (req_has_keep_alive_header_)
            {
                req_keep_alive_.assign((const char *) header.value,
                                        header.value_len);
            }
        }
    }
    // 调用父类的 handle 方法，继续处理任务
    this->WFServerTask::handle(state, error);
}

// 在发送响应前，对响应做一些必须的设置，然后调用父类的发送接口
CommMessageOut *HttpServerTask::message_out()
{
    // 获取当前任务的 HTTP 响应对象
    HttpResp *resp = this->get_resp();

    // 获取响应头的引用
    std::map<std::string, std::string, MapStringCaseLess> &headers = resp->headers;

    // 如果没有设置 Content-Type 头部，设置默认值为 "text/plain"
    if(headers.find("Content-Type") == headers.end())
    {
        headers["Content-Type"] = "text/plain";
    }

    // 如果没有设置 Date 头部，设置为当前时间
    if(headers.find("Date") == headers.end())
    {
        headers["Date"] = Timestamp::now().to_format_str("%a, %d %b %Y %H:%M:%S GMT");
    }

    // 定义一个 HTTP 消息头部结构
    struct HttpMessageHeader header;

    // 遍历所有自定义头部，添加到响应中
    for(auto &header_kv : headers)
    {
        header.name = header_kv.first.c_str();
        header.name_len = header_kv.first.size();
        header.value = header_kv.second.c_str();
        header.value_len = header_kv.second.size();
        resp->protocol::HttpResponse::add_header(&header);
    }

    // 遍历所有 Cookie，添加到响应中
    for(auto &cookie : resp->cookies())
    {
        std::string cookie_str = cookie.dump();
        header.name = "Set-Cookie";
        header.name_len = 10;
        header.value = cookie_str.c_str();
        header.value_len = cookie_str.size();
        resp->protocol::HttpResponse::add_header(&header);
    }

    // 如果没有设置 HTTP 版本，设置默认值为 "HTTP/1.1"
    if (!resp->get_http_version())
        resp->set_http_version("HTTP/1.1");

    // 如果没有设置状态码或原因短语，设置默认值
    const char *status_code_str = resp->get_status_code();
    if (!status_code_str || !resp->get_reason_phrase())
    {
        int status_code;

        if (status_code_str)
            status_code = atoi(status_code_str);
        else
            status_code = HttpStatusOK;

        HttpUtil::set_response_status(resp, status_code);
    }

    // 如果没有设置分块传输编码且没有设置 Content-Length 头部，设置 Content-Length
    if (!resp->is_chunked() && !resp->has_content_length_header())
    {
        char buf[32];
        header.name = "Content-Length";
        header.name_len = strlen("Content-Length");
        header.value = buf;
        header.value_len = sprintf(buf, "%zu", resp->get_output_body_size());
        resp->protocol::HttpResponse::add_header(&header);
    }

    // 确定是否保持连接
    bool is_alive;

    if (resp->has_connection_header())
        is_alive = resp->is_keep_alive();
    else
        is_alive = req_is_alive_;

    if (!is_alive)
        this->keep_alive_timeo = 0;
    else
    {
        // 如果请求包含 Keep-Alive 头部，解析其参数
        if (req_has_keep_alive_header_)
        {
            int flag = 0;
            std::vector<std::string> params = StrUtil::split(req_keep_alive_, ',');

            for (const auto &kv: params)
            {
                std::vector<std::string> arr = StrUtil::split(kv, '=');
                if (arr.size() < 2)
                    arr.emplace_back("0");

                std::string key = StrUtil::strip(arr[0]);
                std::string val = StrUtil::strip(arr[1]);
                if (!(flag & 1) && strcasecmp(key.c_str(), "timeout") == 0)
                {
                    flag |= 1;
                    // 设置 Keep-Alive 超时时间
                    this->keep_alive_timeo = 1000 * atoi(val.c_str());
                    if (flag == 3)
                        break;
                } else if (!(flag & 2) && strcasecmp(key.c_str(), "max") == 0)
                {
                    flag |= 2;
                    // 如果请求的连接数达到最大值，关闭连接
                    if (this->get_seq() >= atoi(val.c_str()))
                    {
                        this->keep_alive_timeo = 0;
                        break;
                    }

                    if (flag == 3)
                        break;
                }
            }
        }

        // 限制 Keep-Alive 超时时间不超过最大值
        if ((unsigned int) this->keep_alive_timeo > HTTP_KEEPALIVE_MAX)
            this->keep_alive_timeo = HTTP_KEEPALIVE_MAX;
    }

    // 如果没有设置 Connection 头部，添加默认值
    if (!resp->has_connection_header())
    {
        header.name = "Connection";
        header.name_len = 10;
        if (this->keep_alive_timeo == 0)
        {
            header.value = "close";
            header.value_len = 5;
        } else
        {
            header.value = "Keep-Alive";
            header.value_len = 10;
        }

        resp->protocol::HttpResponse::add_header(&header);
    }

    // 调用父类的 message_out 方法，完成消息构建
    return this->WFServerTask::message_out();
}

// 获取对端（客户端）的 IP 地址
std::string HttpServerTask::peer_addr() const
{
    // 定义一个足够大的地址结构，用于存储对端地址
    struct sockaddr_storage addr;
    socklen_t addr_len = sizeof(addr); // 地址结构的长度
    // 获取对端地址
    this->get_peer_addr(reinterpret_cast<struct sockaddr *>(&addr), &addr_len);

    // 定义一个静态常量，表示地址字符串的最大长度
    static const int ADDR_STR_LEN = 128;
    char addrstr[ADDR_STR_LEN]; // 用于存储地址字符串
    // 根据地址族类型，将地址转换为字符串形式
    if (addr.ss_family == AF_INET)
    {
        // 如果是 IPv4 地址
        auto *sin = reinterpret_cast<struct sockaddr_in *>(&addr);
        inet_ntop(AF_INET, &sin->sin_addr, addrstr, ADDR_STR_LEN);
    } 
    else if (addr.ss_family == AF_INET6)
    {
        // 如果是 IPv6 地址
        auto *sin6 = reinterpret_cast<struct sockaddr_in6 *>(&addr);
        inet_ntop(AF_INET6, &sin6->sin6_addr, addrstr, ADDR_STR_LEN);
    } 
    else
    {
        // 如果地址族类型未知，返回 "Unknown"
        strcpy(addrstr, "Unknown");
    }

    // 返回地址字符串
    return addrstr;
}

// 获取对端（客户端）的端口号
unsigned short HttpServerTask::peer_port() const
{
    // 定义一个足够大的地址结构，用于存储对端地址
    struct sockaddr_storage addr;
    socklen_t addr_len = sizeof(addr); // 地址结构的长度
    // 获取对端地址
    this->get_peer_addr(reinterpret_cast<struct sockaddr *>(&addr), &addr_len);

    unsigned short port = 0; // 用于存储端口号
    // 根据地址族类型，提取端口号
    if (addr.ss_family == AF_INET)
    {
        // 如果是 IPv4 地址
        auto *sin = reinterpret_cast<struct sockaddr_in *>(&addr);
        port = ntohs(sin->sin_port); // 将网络字节序转换为主机字节序
    } 
    else if (addr.ss_family == AF_INET6)
    {
        // 如果是 IPv6 地址
        auto *sin6 = reinterpret_cast<struct sockaddr_in6 *>(&addr);
        port = ntohs(sin6->sin6_port); // 将网络字节序转换为主机字节序
    }
    // 返回端口号
    return port;
}

// 检查服务器是否设置了关闭标志
bool HttpServerTask::close_flag() const
{
    // 返回服务器的关闭标志状态
    return server->close_flag_;
}

} // namespace Yukino
