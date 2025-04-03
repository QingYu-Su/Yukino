#ifndef YUKINO_HTTPSERVER_H_
#define YUKINO_HTTPSERVER_H_

#include "workflow/WFHttpServer.h"
#include "workflow/HttpUtil.h"

#include <unordered_map>
#include <string>

#include "HttpMsg.h"
#include "BluePrint.h"

namespace Yukino
{

class HttpServer : public WFServer<HttpReq, HttpResp>, public Noncopyable
{
public:
    // 注册路由，支持单一HTTP方法
    void ROUTE(const std::string &route, const Handler &handler, Verb verb)
    {
        // 调用内部 BluePrint 对象的 ROUTE 方法
        blue_print_.ROUTE(route, handler, verb);
    }

    // 注册路由，支持单一HTTP方法，并指定计算队列ID
    void ROUTE(const std::string &route, int compute_queue_id, const Handler &handler, Verb verb)
    {
        // 调用内部 BluePrint 对象的 ROUTE 方法
        blue_print_.ROUTE(route, compute_queue_id, handler, verb);
    }

    // 注册路由，支持多个HTTP方法（通过方法名字符串列表指定）
    void ROUTE(const std::string &route, const Handler &handler, const std::vector<std::string> &methods)
    {
        // 调用内部 BluePrint 对象的 ROUTE 方法
        blue_print_.ROUTE(route, handler, methods);
    }

    // 注册路由，支持多个HTTP方法，并指定计算队列ID
    void ROUTE(const std::string &route, int compute_queue_id,
            const Handler &handler, const std::vector<std::string> &methods)
    {
        // 调用内部 BluePrint 对象的 ROUTE 方法
        blue_print_.ROUTE(route, compute_queue_id, handler, methods);
    }

    // 注册GET请求的路由
    void GET(const std::string &route, const Handler &handler)
    {
        // 调用内部 BluePrint 对象的 GET 方法
        blue_print_.GET(route, handler);
    }

    // 注册GET请求的路由，并指定计算队列ID
    void GET(const std::string &route, int compute_queue_id, const Handler &handler)
    {
        // 调用内部 BluePrint 对象的 GET 方法
        blue_print_.GET(route, compute_queue_id, handler);
    }

    // 注册POST请求的路由
    void POST(const std::string &route, const Handler &handler)
    {
        // 调用内部 BluePrint 对象的 POST 方法
        blue_print_.POST(route, handler);
    }

    // 注册POST请求的路由，并指定计算队列ID
    void POST(const std::string &route, int compute_queue_id, const Handler &handler)
    {
        // 调用内部 BluePrint 对象的 POST 方法
        blue_print_.POST(route, compute_queue_id, handler);
    }

    // 注册DELETE请求的路由
    void DELETE(const std::string &route, const Handler &handler)
    {
        // 调用内部 BluePrint 对象的 DELETE 方法
        blue_print_.DELETE(route, handler);
    }

    // 注册DELETE请求的路由，并指定计算队列ID
    void DELETE(const std::string &route, int compute_queue_id, const Handler &handler)
    {
        // 调用内部 BluePrint 对象的 DELETE 方法
        blue_print_.DELETE(route, compute_queue_id, handler);
    }

    // 注册PATCH请求的路由
    void PATCH(const std::string &route, const Handler &handler)
    {
        // 调用内部 BluePrint 对象的 PATCH 方法
        blue_print_.PATCH(route, handler);
    }

    // 注册PATCH请求的路由，并指定计算队列ID
    void PATCH(const std::string &route, int compute_queue_id, const Handler &handler)
    {
        // 调用内部 BluePrint 对象的 PATCH 方法
        blue_print_.PATCH(route, compute_queue_id, handler);
    }

    // 注册PUT请求的路由
    void PUT(const std::string &route, const Handler &handler)
    {
        // 调用内部 BluePrint 对象的 PUT 方法
        blue_print_.PUT(route, handler);
    }

    // 注册PUT请求的路由，并指定计算队列ID
    void PUT(const std::string &route, int compute_queue_id, const Handler &handler)
    {
        // 调用内部 BluePrint 对象的 PUT 方法
        blue_print_.PUT(route, compute_queue_id, handler);
    }

    // 注册HEAD请求的路由
    void HEAD(const std::string &route, const Handler &handler)
    {
        // 调用内部 BluePrint 对象的 HEAD 方法
        blue_print_.HEAD(route, handler);
    }

    // 注册HEAD请求的路由，并指定计算队列ID
    void HEAD(const std::string &route, int compute_queue_id, const Handler &handler)
    {
        // 调用内部 BluePrint 对象的 HEAD 方法
        blue_print_.HEAD(route, compute_queue_id, handler);
    }

public:
    // 模板函数，用于注册路由，支持单一HTTP方法，并允许传递额外的参数
    template<typename... AP>
    void ROUTE(const std::string &route, const Handler &handler,
            Verb verb, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 ROUTE 方法
        blue_print_.ROUTE(route, handler, verb, ap...);
    }

    // 模板函数，用于注册路由，支持单一HTTP方法，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void ROUTE(const std::string &route, int compute_queue_id,
            const Handler &handler, Verb verb, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 ROUTE 方法
        blue_print_.ROUTE(route, compute_queue_id, handler, verb, ap...);
    }

    // 模板函数，用于注册路由，支持多个HTTP方法（通过方法名字符串列表指定），并允许传递额外的参数
    template<typename... AP>
    void ROUTE(const std::string &route, const Handler &handler,
            const std::vector<std::string> &methods, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 ROUTE 方法
        blue_print_.ROUTE(route, handler, methods, ap...);
    }

    // 模板函数，用于注册路由，支持多个HTTP方法，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void ROUTE(const std::string &route, int compute_queue_id,
            const Handler &handler,
            const std::vector<std::string> &methods, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 ROUTE 方法
        blue_print_.ROUTE(route, compute_queue_id, handler, methods, ap...);
    }

    // 模板函数，用于注册GET请求的路由，并允许传递额外的参数
    template<typename... AP>
    void GET(const std::string &route, const Handler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 GET 方法
        blue_print_.GET(route, handler, ap...);
    }

    // 模板函数，用于注册GET请求的路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void GET(const std::string &route, int compute_queue_id,
            const Handler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 GET 方法
        blue_print_.GET(route, compute_queue_id, handler, ap...);
    }

    // 模板函数，用于注册POST请求的路由，并允许传递额外的参数
    template<typename... AP>
    void POST(const std::string &route, const Handler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 POST 方法
        blue_print_.POST(route, handler, ap...);
    }

    // 模板函数，用于注册POST请求的路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void POST(const std::string &route, int compute_queue_id,
            const Handler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 POST 方法
        blue_print_.POST(route, compute_queue_id, handler, ap...);
    }

    // 模板函数，用于注册DELETE请求的路由，并允许传递额外的参数
    template<typename... AP>
    void DELETE(const std::string &route, const Handler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 DELETE 方法
        blue_print_.DELETE(route, handler, ap...);
    }

    // 模板函数，用于注册DELETE请求的路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void DELETE(const std::string &route, int compute_queue_id,
                const Handler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 DELETE 方法
        blue_print_.DELETE(route, compute_queue_id, handler, ap...);
    }

    // 模板函数，用于注册PATCH请求的路由，并允许传递额外的参数
    template<typename... AP>
    void PATCH(const std::string &route, const Handler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 PATCH 方法
        blue_print_.PATCH(route, handler, ap...);
    }

    // 模板函数，用于注册PATCH请求的路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void PATCH(const std::string &route, int compute_queue_id,
            const Handler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 PATCH 方法
        blue_print_.PATCH(route, compute_queue_id, handler, ap...);
    }

    // 模板函数，用于注册PUT请求的路由，并允许传递额外的参数
    template<typename... AP>
    void PUT(const std::string &route, const Handler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 PUT 方法
        blue_print_.PUT(route, handler, ap...);
    }

    // 模板函数，用于注册PUT请求的路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void PUT(const std::string &route, int compute_queue_id,
            const Handler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 PUT 方法
        blue_print_.PUT(route, compute_queue_id, handler, ap...);
    }

    // 模板函数，用于注册HEAD请求的路由，并允许传递额外的参数
    template<typename... AP>
    void HEAD(const std::string &route, const Handler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 HEAD 方法
        blue_print_.HEAD(route, handler, ap...);
    }

    // 模板函数，用于注册HEAD请求的路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void HEAD(const std::string &route, int compute_queue_id,
            const Handler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 HEAD 方法
        blue_print_.HEAD(route, compute_queue_id, handler, ap...);
    }

public:
    // 注册路由，支持单一HTTP方法
    void ROUTE(const std::string &route, const SeriesHandler &handler, Verb verb)
    {
        // 调用内部 BluePrint 对象的 ROUTE 方法
        blue_print_.ROUTE(route, handler, verb);
    }

    // 注册路由，支持单一HTTP方法，并指定计算队列ID
    void ROUTE(const std::string &route, int compute_queue_id, const SeriesHandler &handler, Verb verb)
    {
        // 调用内部 BluePrint 对象的 ROUTE 方法
        blue_print_.ROUTE(route, compute_queue_id, handler, verb);
    }

    // 注册路由，支持多个HTTP方法（通过方法名字符串列表指定）
    void ROUTE(const std::string &route, const SeriesHandler &handler, const std::vector<std::string> &methods)
    {
        // 调用内部 BluePrint 对象的 ROUTE 方法
        blue_print_.ROUTE(route, handler, methods);
    }

    // 注册路由，支持多个HTTP方法，并指定计算队列ID
    void ROUTE(const std::string &route, int compute_queue_id,
            const SeriesHandler &handler, const std::vector<std::string> &methods)
    {
        // 调用内部 BluePrint 对象的 ROUTE 方法
        blue_print_.ROUTE(route, compute_queue_id, handler, methods);
    }

    // 注册GET请求的路由
    void GET(const std::string &route, const SeriesHandler &handler)
    {
        // 调用内部 BluePrint 对象的 GET 方法
        blue_print_.GET(route, handler);
    }

    // 注册GET请求的路由，并指定计算队列ID
    void GET(const std::string &route, int compute_queue_id, const SeriesHandler &handler)
    {
        // 调用内部 BluePrint 对象的 GET 方法
        blue_print_.GET(route, compute_queue_id, handler);
    }

    // 注册POST请求的路由
    void POST(const std::string &route, const SeriesHandler &handler)
    {
        // 调用内部 BluePrint 对象的 POST 方法
        blue_print_.POST(route, handler);
    }

    // 注册POST请求的路由，并指定计算队列ID
    void POST(const std::string &route, int compute_queue_id, const SeriesHandler &handler)
    {
        // 调用内部 BluePrint 对象的 POST 方法
        blue_print_.POST(route, compute_queue_id, handler);
    }

    // 注册DELETE请求的路由
    void DELETE(const std::string &route, const SeriesHandler &handler)
    {
        // 调用内部 BluePrint 对象的 DELETE 方法
        blue_print_.DELETE(route, handler);
    }

    // 注册DELETE请求的路由，并指定计算队列ID
    void DELETE(const std::string &route, int compute_queue_id, const SeriesHandler &handler)
    {
        // 调用内部 BluePrint 对象的 DELETE 方法
        blue_print_.DELETE(route, compute_queue_id, handler);
    }

    // 注册PATCH请求的路由
    void PATCH(const std::string &route, const SeriesHandler &handler)
    {
        // 调用内部 BluePrint 对象的 PATCH 方法
        blue_print_.PATCH(route, handler);
    }

    // 注册PATCH请求的路由，并指定计算队列ID
    void PATCH(const std::string &route, int compute_queue_id, const SeriesHandler &handler)
    {
        // 调用内部 BluePrint 对象的 PATCH 方法
        blue_print_.PATCH(route, compute_queue_id, handler);
    }

    // 注册PUT请求的路由
    void PUT(const std::string &route, const SeriesHandler &handler)
    {
        // 调用内部 BluePrint 对象的 PUT 方法
        blue_print_.PUT(route, handler);
    }

    // 注册PUT请求的路由，并指定计算队列ID
    void PUT(const std::string &route, int compute_queue_id, const SeriesHandler &handler)
    {
        // 调用内部 BluePrint 对象的 PUT 方法
        blue_print_.PUT(route, compute_queue_id, handler);
    }

    // 注册HEAD请求的路由
    void HEAD(const std::string &route, const SeriesHandler &handler)
    {
        // 调用内部 BluePrint 对象的 HEAD 方法
        blue_print_.HEAD(route, handler);
    }

    // 注册HEAD请求的路由，并指定计算队列ID
    void HEAD(const std::string &route, int compute_queue_id, const SeriesHandler &handler)
    {
        // 调用内部 BluePrint 对象的 HEAD 方法
        blue_print_.HEAD(route, compute_queue_id, handler);
    }

public:
    // 模板函数，用于注册路由，支持单一HTTP方法，并允许传递额外的参数
    template<typename... AP>
    void ROUTE(const std::string &route, const SeriesHandler &handler,
            Verb verb, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 ROUTE 方法
        blue_print_.ROUTE(route, handler, verb, ap...);
    }

    // 模板函数，用于注册路由，支持单一HTTP方法，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void ROUTE(const std::string &route, int compute_queue_id,
            const SeriesHandler &handler, Verb verb, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 ROUTE 方法
        blue_print_.ROUTE(route, compute_queue_id, handler, verb, ap...);
    }

    // 模板函数，用于注册路由，支持多个HTTP方法（通过方法名字符串列表指定），并允许传递额外的参数
    template<typename... AP>
    void ROUTE(const std::string &route, const SeriesHandler &handler,
            const std::vector<std::string> &methods, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 ROUTE 方法
        blue_print_.ROUTE(route, handler, methods, ap...);
    }

    // 模板函数，用于注册路由，支持多个HTTP方法，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void ROUTE(const std::string &route, int compute_queue_id,
            const SeriesHandler &handler,
            const std::vector<std::string> &methods, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 ROUTE 方法
        blue_print_.ROUTE(route, compute_queue_id, handler, methods, ap...);
    }

    // 模板函数，用于注册GET请求的路由，并允许传递额外的参数
    template<typename... AP>
    void GET(const std::string &route, const SeriesHandler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 GET 方法
        blue_print_.GET(route, handler, ap...);
    }

    // 模板函数，用于注册GET请求的路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void GET(const std::string &route, int compute_queue_id,
            const SeriesHandler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 GET 方法
        blue_print_.GET(route, compute_queue_id, handler, ap...);
    }

    // 模板函数，用于注册POST请求的路由，并允许传递额外的参数
    template<typename... AP>
    void POST(const std::string &route, const SeriesHandler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 POST 方法
        blue_print_.POST(route, handler, ap...);
    }

    // 模板函数，用于注册POST请求的路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void POST(const std::string &route, int compute_queue_id,
            const SeriesHandler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 POST 方法
        blue_print_.POST(route, compute_queue_id, handler, ap...);
    }

    // 模板函数，用于注册DELETE请求的路由，并允许传递额外的参数
    template<typename... AP>
    void DELETE(const std::string &route, const SeriesHandler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 DELETE 方法
        blue_print_.DELETE(route, handler, ap...);
    }

    // 模板函数，用于注册DELETE请求的路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void DELETE(const std::string &route, int compute_queue_id,
                const SeriesHandler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 DELETE 方法
        blue_print_.DELETE(route, compute_queue_id, handler, ap...);
    }

    // 模板函数，用于注册PATCH请求的路由，并允许传递额外的参数
    template<typename... AP>
    void PATCH(const std::string &route, const SeriesHandler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 PATCH 方法
        blue_print_.PATCH(route, handler, ap...);
    }

    // 模板函数，用于注册PATCH请求的路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void PATCH(const std::string &route, int compute_queue_id,
            const SeriesHandler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 PATCH 方法
        blue_print_.PATCH(route, compute_queue_id, handler, ap...);
    }

    // 模板函数，用于注册PUT请求的路由，并允许传递额外的参数
    template<typename... AP>
    void PUT(const std::string &route, const SeriesHandler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 PUT 方法
        blue_print_.PUT(route, handler, ap...);
    }

    // 模板函数，用于注册PUT请求的路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void PUT(const std::string &route, int compute_queue_id,
            const SeriesHandler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 PUT 方法
        blue_print_.PUT(route, compute_queue_id, handler, ap...);
    }

    // 模板函数，用于注册HEAD请求的路由，并允许传递额外的参数
    template<typename... AP>
    void HEAD(const std::string &route, const SeriesHandler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 HEAD 方法
        blue_print_.HEAD(route, handler, ap...);
    }

    // 模板函数，用于注册HEAD请求的路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void HEAD(const std::string &route, int compute_queue_id,
            const SeriesHandler &handler, const AP &... ap)
    {
        // 调用内部 BluePrint 对象的 HEAD 方法
        blue_print_.HEAD(route, compute_queue_id, handler, ap...);
    }

public:
    // 声明 HttpServerTask 为友元类，允许 HttpServerTask 访问 HttpServer 的私有成员
    friend class HttpServerTask;

    // 静态文件服务，指定相对路径和根目录
    void Static(const char *relative_path, const char *root);

    // 列出所有注册的路由
    void list_routes();

    // 设置默认路由
    void set_default_route(const std::string& default_route)
    {
        default_route_ = default_route;
    }

    // 注册一个 BluePrint 对象，并指定 URL 前缀
    void register_blueprint(const BluePrint &bp, const std::string &url_prefix);

    // 注册中间件或全局切面
    template <typename... AP>
    void Use(AP &&...ap)
    {
        // 创建一个包含可变参数的元组
        auto *tp = new std::tuple<AP...>(std::move(ap)...);
        // 遍历元组中的每个元素，并应用全局切面函数
        for_each(*tp, GlobalAspectFunc());
    }

    // 停止服务器
    void stop()
    {
        close_flag_ = true; // 设置关闭标志
        WFServerBase::stop(); // 调用基类的 stop 方法
    }

public:
    // HttpServer 类的构造函数
    HttpServer() :
    WFServer(std::bind(&HttpServer::process, this, std::placeholders::_1))
    {}

    // 设置最大连接数
    HttpServer &max_connections(size_t max_connections)
    {
    this->params.max_connections = max_connections;
    return *this;
    }

    // 设置对端响应超时时间
    HttpServer &peer_response_timeout(int peer_response_timeout)
    {
    this->params.peer_response_timeout = peer_response_timeout;
    return *this;
    }

    // 设置接收超时时间
    HttpServer &receive_timeout(int receive_timeout)
    {
    this->params.receive_timeout = receive_timeout;
    return *this;
    }

    // 设置 Keep-Alive 超时时间
    HttpServer &keep_alive_timeout(int keep_alive_timeout)
    {
    this->params.keep_alive_timeout = keep_alive_timeout;
    return *this;
    }

    // 设置请求大小限制
    HttpServer &request_size_limit(size_t request_size_limit)
    {
    this->params.request_size_limit = request_size_limit;
    return *this;
    }

    // 设置 SSL 接受超时时间
    HttpServer &ssl_accept_timeout(int ssl_accept_timeout)
    {
    this->params.ssl_accept_timeout = ssl_accept_timeout;
    return *this;
    }

    // 使用 TrackFunc 类型的跟踪函数
    using TrackFunc = std::function<void(HttpTask *server_task)>;

    // 设置跟踪函数（无参数版本），用来打印日志
    HttpServer &track();

    // 设置跟踪函数（接受 TrackFunc 类型的引用），它会在HttpServerTask完成时被调用
    HttpServer &track(const TrackFunc &track_func);

    // 设置跟踪函数（接受 TrackFunc 类型的右值引用），它会在HttpServerTask完成时被调用
    HttpServer &track(TrackFunc &&track_func);

    // 打印路由树结构（用于测试）
    void print_node_arch() { blue_print_.print_node_arch(); }

protected:
    // 创建新的会话（重写自基类）
    CommSession *new_session(long long seq, CommConnection *conn) override;

    private:
    // 处理 HTTP 任务
    void process(HttpTask *task);

    // 为静态文件服务提供支持
    int serve_static(const char *path, BluePrint &bp);

    // 全局切面函数
    struct GlobalAspectFunc
    {
        template <typename T>
        void operator()(T &t) const
        {
            Aspect *asp = new T(std::move(t));
            GlobalAspect *global_aspect = GlobalAspect::get_instance();
            global_aspect->aspect_list.push_back(asp);
        }
    };

private:
    bool close_flag_ = false; // 关闭标志
    std::string default_route_; // 默认路由
    BluePrint blue_print_; // 内部 BluePrint 对象
    TrackFunc track_func_; // 跟踪函数
};

}  // namespace Yukino

#endif // YUKINO_HTTPSERVER_H_
