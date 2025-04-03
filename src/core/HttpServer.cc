#include "workflow/HttpMessage.h"

#include <utility>

#include "HttpServer.h"
#include "HttpServerTask.h"
#include "UriUtil.h"
#include "HttpFile.h"
#include "PathUtil.h"
#include "Router.h"
#include "ErrorCode.h"
#include "CodeUtil.h"
#include "spdlog/spdlog.h" 

using namespace Yukino;

void HttpServer::process(HttpTask *task)
{
    // 将 HttpTask 转换为 HttpServerTask
    auto *server_task = static_cast<HttpServerTask *>(task);
    server_task->server = this; // 设置当前服务器对象
    auto *req = server_task->get_req(); // 获取请求对象
    auto *resp = server_task->get_resp(); // 获取响应对象
    const char *request_uri;
    std::string uri_str;

    // 填充请求头和内容类型
    req->fill_header_map();
    req->fill_content_type();

    // 获取 Host 头部
    const std::string &host = req->header("Host");

    // 检查 Host 头部是否有效
    if (host.empty() || host.find_first_of("/?#") != std::string::npos)
    {
        resp->set_status(HttpStatusBadRequest); // 设置状态码为 400 Bad Request
        return;
    }

    // 获取请求 URI
    request_uri = req->get_request_uri();
    if (strncasecmp(request_uri, "http://", 7) == 0 ||
        strncasecmp(request_uri, "https://", 8) == 0)
    {
        // 如果请求 URI 以 http:// 或 https:// 开头，直接使用
        uri_str = request_uri;
    }
    else if (*request_uri == '/')
    {
        // 如果请求 URI 以 / 开头，拼接完整的 URI
        const char *scheme = this->get_ssl_ctx() ? "https://" : "http://";
        uri_str = scheme + host + req->get_request_uri();
    }
    else
    {
        // 如果请求 URI 不合法，返回 400 Bad Request
        resp->set_status(HttpStatusBadRequest);
        return;
    }

    // 解析 URI
    ParsedURI uri;
    if (URIParser::parse(uri_str, uri) < 0)
    {
        // 如果解析失败，返回 400 Bad Request
        resp->set_status(HttpStatusBadRequest);
        return;
    }
    if (!uri.path)
        uri.path = strdup("/"); // 如果路径为空，设置为 "/"

    // 处理路径中的 "." 和 ".."
    std::string route("/");
    const char *pos = uri.path;
    while (*pos)
    {
        const char *slash = pos;
        while (*slash && *slash != '/')
            slash++;

        size_t n = slash - pos;
        if (n == 0 || (n == 1 && *pos == '.'))
            ; // 忽略单个 "."
        else if (n == 2 && *pos == '.' && pos[1] == '.')
        {
            // 处理 ".."，回退到上一级目录
            if (route.size() > 1)
            {
                n = route.find_last_of('/', route.size() - 2);
                route.resize(n + 1);
            }
        }
        else
        {
            // 添加路径段
            route.append(pos, slash);
            if (*slash)
                route.push_back('/');
        }

        if (!*slash)
            break;

        pos = slash + 1;
    }

    // 处理查询参数
    if (uri.query)
    {
        StringPiece query(uri.query);
        req->set_query_params(UriUtil::split_query(query));
    }

    // 设置解析后的 URI
    req->set_parsed_uri(std::move(uri));
    std::string verb = req->get_method(); // 获取 HTTP 方法
    int ret = blue_print_.router().call(str_to_verb(verb), CodeUtil::url_encode(route), server_task);
    if(ret != StatusOK && !default_route_.empty())
    {
        // 如果路由匹配失败且设置了默认路由，尝试匹配默认路由
        ret = blue_print_.router().call(str_to_verb(verb), CodeUtil::url_encode(default_route_), server_task);
    }
    if (ret != StatusOK) {
        // 如果路由匹配失败，返回错误响应
        resp->Error(ret, verb + " " + route);
    }
    if(track_func_)
    {
        // 如果设置了跟踪函数，添加到任务的回调中
        server_task->add_callback(track_func_);
    }
}

// 创建新的会话
CommSession *HttpServer::new_session(long long seq, CommConnection *conn)
{
    // 创建一个新的 HttpServerTask 对象
    HttpTask *task = new HttpServerTask(this, this->WFServer<HttpReq, HttpResp>::process);
    // 设置任务的 Keep-Alive 超时时间
    task->set_keep_alive(this->params.keep_alive_timeout);
    // 设置任务的接收超时时间
    task->set_receive_timeout(this->params.receive_timeout);
    // 设置请求的最大大小限制
    task->get_req()->set_size_limit(this->params.request_size_limit);

    return task;
}

// 列出所有注册的路由
void HttpServer::list_routes()
{
    // 调用内部 BluePrint 对象的 print_routes 方法
    blue_print_.router().print_routes();
}

// 注册一个 BluePrint 对象，并指定 URL 前缀
void HttpServer::register_blueprint(const BluePrint &bp, const std::string& url_prefix)
{
    // 调用内部 BluePrint 对象的 add_blueprint 方法
    blue_print_.add_blueprint(bp, url_prefix);
}

// 静态文件服务
void HttpServer::Static(const char *relative_path, const char *root)
{
    BluePrint bp;
    // 调用 serve_static 方法，尝试为静态文件服务提供支持
    int ret = serve_static(root, bp);
    if(ret != StatusOK)
    {
        // 如果服务失败，打印错误信息
        spdlog::error("[YUKINO] {} dose not exists", root);
        return;
    }
    // 将静态文件服务的蓝图添加到服务器中
    blue_print_.add_blueprint(std::move(bp), relative_path);
}

// 为静态文件服务提供支持
int HttpServer::serve_static(const char* path, BluePrint &bp)
{
    std::string path_str(path);
    bool is_file = true;
    // 检查路径是否为目录
    if (PathUtil::is_dir(path_str))
    {
        is_file = false;
    } 
    else if(!PathUtil::is_file(path_str))
    {
        // 如果路径既不是文件也不是目录，返回 StatusNotFound
        return StatusNotFound;
    }
    std::string route = "";
    if (!is_file)
    {
        // 如果路径是目录，设置路由为 "/*"
        route = "/*";
    }
    // 注册 GET 请求的处理函数
    bp.GET(route, [path_str, is_file](const HttpReq *req, HttpResp *resp) {
        std::string match_path = req->match_path();
        if(is_file)
        {
            // 如果路径是文件，直接返回文件内容
            resp->File(path_str);
        } 
        else
        {
            // 如果路径是目录，返回目录下的文件内容
            resp->File(path_str + "/" + match_path);
        }
    });
    return StatusOK;
}

// 设置默认的跟踪函数
HttpServer &HttpServer::track()
{
    // 设置一个默认的跟踪函数，用于打印请求和响应信息
    track_func_ = [](HttpTask *server_task) {
        HttpResp *resp = server_task->get_resp();
        HttpReq *req = server_task->get_req();
        HttpServerTask *task = static_cast<HttpServerTask *>(server_task);
        Timestamp current_time = Timestamp::now();
        std::string fmt_time = current_time.to_format_str();
        // 打印请求和响应信息
        spdlog::info("[YUKINO] {0} | {1} | {2} : {3} | {4} | \"{5}\" | --",
                    fmt_time.c_str(),
                    resp->get_status_code(),
                    task->peer_addr().c_str(),
                    task->peer_port(),
                    req->get_method(),
                    req->current_path().c_str());
    };
    return *this;
}

// 设置自定义的跟踪函数（接受引用）
HttpServer &HttpServer::track(const TrackFunc &track_func)
{
    track_func_ = track_func;
    return *this;
}

// 设置自定义的跟踪函数（接受右值引用）
HttpServer &HttpServer::track(TrackFunc &&track_func)
{
    track_func_ = std::move(track_func);
    return *this;
}

