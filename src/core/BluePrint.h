#ifndef YUKINO_BLUEPRINT_H_
#define YUKINO_BLUEPRINT_H_

#include <functional>
#include <utility>
#include "Noncopyable.h"
#include "Aspect.h"
#include "AopUtil.h"

// todo : hide
#include "Router.h"
#include "HttpServerTask.h" 

class SeriesWork;
namespace Yukino
{
    // 定义一个普通的请求处理函数类型
    using Handler = std::function<void(const HttpReq *, HttpResp *)>;
    // 定义一个支持异步序列化任务的请求处理函数类型
    using SeriesHandler = std::function<void(const HttpReq *, HttpResp *, SeriesWork *)>;

class BluePrint : public Noncopyable
{
public:
    // 注册路由的基本接口，支持单一HTTP方法
    void ROUTE(const std::string &route, const Handler &handler, Verb verb);
    // 注册路由的基本接口，支持单一HTTP方法，并指定计算队列ID
    void ROUTE(const std::string &route, int compute_queue_id, const Handler &handler, Verb verb);

    // 注册路由的基本接口，支持多个HTTP方法（通过方法名字符串列表指定）
    void ROUTE(const std::string &route, const Handler &handler, const std::vector<std::string> &methods);
    // 注册路由的基本接口，支持多个HTTP方法，并指定计算队列ID
    void ROUTE(const std::string &route, int compute_queue_id, 
                const Handler &handler, const std::vector<std::string> &methods);

    // 注册GET请求的路由
    void GET(const std::string &route, const Handler &handler);
    // 注册GET请求的路由，并指定计算队列ID
    void GET(const std::string &route, int compute_queue_id, const Handler &handler);

    // 注册POST请求的路由
    void POST(const std::string &route, const Handler &handler);
    // 注册POST请求的路由，并指定计算队列ID
    void POST(const std::string &route, int compute_queue_id, const Handler &handler);

    // 注册DELETE请求的路由
    void DELETE(const std::string &route, const Handler &handler);
    // 注册DELETE请求的路由，并指定计算队列ID
    void DELETE(const std::string &route, int compute_queue_id, const Handler &handler);

    // 注册PATCH请求的路由
    void PATCH(const std::string &route, const Handler &handler);
    // 注册PATCH请求的路由，并指定计算队列ID
    void PATCH(const std::string &route, int compute_queue_id, const Handler &handler);

    // 注册PUT请求的路由
    void PUT(const std::string &route, const Handler &handler);
    // 注册PUT请求的路由，并指定计算队列ID
    void PUT(const std::string &route, int compute_queue_id, const Handler &handler);

    // 注册HEAD请求的路由
    void HEAD(const std::string &route, const Handler &handler);
    // 注册HEAD请求的路由，并指定计算队列ID
    void HEAD(const std::string &route, int compute_queue_id, const Handler &handler);

public:
    // 模板函数，用于注册路由，支持单一HTTP方法，并允许传递额外的参数（如切面、中间件等）
    template<typename... AP>
    void ROUTE(const std::string &route, const Handler &handler, 
            Verb verb, const AP &... ap);
    // 模板函数，用于注册路由，支持单一HTTP方法，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void ROUTE(const std::string &route, int compute_queue_id, 
            const Handler &handler, Verb verb, const AP &... ap);

    // 模板函数，用于注册路由，支持多个HTTP方法（通过方法名字符串列表指定），并允许传递额外的参数
    template<typename... AP>
    void ROUTE(const std::string &route, const Handler &handler, 
            const std::vector<std::string> &methods, const AP &... ap);
    // 模板函数，用于注册路由，支持多个HTTP方法，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void ROUTE(const std::string &route, int compute_queue_id, 
            const Handler &handler, 
            const std::vector<std::string> &methods, const AP &... ap);

    // 模板函数，用于注册GET请求的路由，并允许传递额外的参数
    template<typename... AP>
    void GET(const std::string &route, const Handler &handler, const AP &... ap);
    // 模板函数，用于注册GET请求的路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void GET(const std::string &route, int compute_queue_id,
            const Handler &handler, const AP &... ap);

    // 模板函数，用于注册POST请求的路由，并允许传递额外的参数
    template<typename... AP>
    void POST(const std::string &route, const Handler &handler, const AP &... ap);
    // 模板函数，用于注册POST请求的路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void POST(const std::string &route, int compute_queue_id,
            const Handler &handler, const AP &... ap);

    // 模板函数，用于注册DELETE请求的路由，并允许传递额外的参数
    template<typename... AP>
    void DELETE(const std::string &route, const Handler &handler, const AP &... ap);
    // 模板函数，用于注册DELETE请求的路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void DELETE(const std::string &route, int compute_queue_id,
                const Handler &handler, const AP &... ap);

    // 模板函数，用于注册PATCH请求的路由，并允许传递额外的参数
    template<typename... AP>
    void PATCH(const std::string &route, const Handler &handler, const AP &... ap);
    // 模板函数，用于注册PATCH请求的路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void PATCH(const std::string &route, int compute_queue_id,
            const Handler &handler, const AP &... ap);

    // 模板函数，用于注册PUT请求的路由，并允许传递额外的参数
    template<typename... AP>
    void PUT(const std::string &route, const Handler &handler, const AP &... ap);
    // 模板函数，用于注册PUT请求的路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void PUT(const std::string &route, int compute_queue_id,
            const Handler &handler, const AP &... ap);

    // 模板函数，用于注册HEAD请求的路由，并允许传递额外的参数
    template<typename... AP>
    void HEAD(const std::string &route, const Handler &handler, const AP &... ap);
    // 模板函数，用于注册HEAD请求的路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void HEAD(const std::string &route, int compute_queue_id,
            const Handler &handler, const AP &... ap);

public:
    // 注册一个支持异步序列化任务的路由，支持单一HTTP方法
    void ROUTE(const std::string &route, const SeriesHandler &handler, Verb verb);

    // 注册一个支持异步序列化任务的路由，支持单一HTTP方法，并指定计算队列ID
    void ROUTE(const std::string &route, int compute_queue_id, const SeriesHandler &handler, Verb verb);

    // 注册一个支持异步序列化任务的路由，支持多个HTTP方法（通过方法名字符串列表指定）
    void ROUTE(const std::string &route, const SeriesHandler &handler, const std::vector<std::string> &methods);

    // 注册一个支持异步序列化任务的路由，支持多个HTTP方法，并指定计算队列ID
    void ROUTE(const std::string &route, int compute_queue_id, 
            const SeriesHandler &handler, const std::vector<std::string> &methods);

    // 注册一个支持异步序列化任务的GET请求路由
    void GET(const std::string &route, const SeriesHandler &handler);

    // 注册一个支持异步序列化任务的GET请求路由，并指定计算队列ID
    void GET(const std::string &route, int compute_queue_id, const SeriesHandler &handler);

    // 注册一个支持异步序列化任务的POST请求路由
    void POST(const std::string &route, const SeriesHandler &handler);

    // 注册一个支持异步序列化任务的POST请求路由，并指定计算队列ID
    void POST(const std::string &route, int compute_queue_id, const SeriesHandler &handler);

    // 注册一个支持异步序列化任务的DELETE请求路由
    void DELETE(const std::string &route, const SeriesHandler &handler);

    // 注册一个支持异步序列化任务的DELETE请求路由，并指定计算队列ID
    void DELETE(const std::string &route, int compute_queue_id, const SeriesHandler &handler);

    // 注册一个支持异步序列化任务的PATCH请求路由
    void PATCH(const std::string &route, const SeriesHandler &handler);

    // 注册一个支持异步序列化任务的PATCH请求路由，并指定计算队列ID
    void PATCH(const std::string &route, int compute_queue_id, const SeriesHandler &handler);

    // 注册一个支持异步序列化任务的PUT请求路由
    void PUT(const std::string &route, const SeriesHandler &handler);

    // 注册一个支持异步序列化任务的PUT请求路由，并指定计算队列ID
    void PUT(const std::string &route, int compute_queue_id, const SeriesHandler &handler);

    // 注册一个支持异步序列化任务的HEAD请求路由
    void HEAD(const std::string &route, const SeriesHandler &handler);

    // 注册一个支持异步序列化任务的HEAD请求路由，并指定计算队列ID
    void HEAD(const std::string &route, int compute_queue_id, const SeriesHandler &handler);

public:
    // 模板函数，用于注册支持异步序列化任务的路由，支持单一HTTP方法，并允许传递额外的参数
    template<typename... AP>
    void ROUTE(const std::string &route, const SeriesHandler &handler, 
            Verb verb, const AP &... ap);
    // 模板函数，用于注册支持异步序列化任务的路由，支持单一HTTP方法，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void ROUTE(const std::string &route, int compute_queue_id, 
            const SeriesHandler &handler, Verb verb, const AP &... ap);

    // 模板函数，用于注册支持异步序列化任务的路由，支持多个HTTP方法（通过方法名字符串列表指定），并允许传递额外的参数
    template<typename... AP>
    void ROUTE(const std::string &route, const SeriesHandler &handler, 
            const std::vector<std::string> &methods, const AP &... ap);
    // 模板函数，用于注册支持异步序列化任务的路由，支持多个HTTP方法，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void ROUTE(const std::string &route, int compute_queue_id, 
            const SeriesHandler &handler, 
            const std::vector<std::string> &methods, const AP &... ap);

    // 模板函数，用于注册支持异步序列化任务的GET请求路由，并允许传递额外的参数
    template<typename... AP>
    void GET(const std::string &route, const SeriesHandler &handler, const AP &... ap);
    // 模板函数，用于注册支持异步序列化任务的GET请求路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void GET(const std::string &route, int compute_queue_id,
            const SeriesHandler &handler, const AP &... ap);

    // 模板函数，用于注册支持异步序列化任务的POST请求路由，并允许传递额外的参数
    template<typename... AP>
    void POST(const std::string &route, const SeriesHandler &handler, const AP &... ap);
    // 模板函数，用于注册支持异步序列化任务的POST请求路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void POST(const std::string &route, int compute_queue_id,
            const SeriesHandler &handler, const AP &... ap);

    // 模板函数，用于注册支持异步序列化任务的DELETE请求路由，并允许传递额外的参数
    template<typename... AP>
    void DELETE(const std::string &route, const SeriesHandler &handler, const AP &... ap);
    // 模板函数，用于注册支持异步序列化任务的DELETE请求路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void DELETE(const std::string &route, int compute_queue_id,
                const SeriesHandler &handler, const AP &... ap);

    // 模板函数，用于注册支持异步序列化任务的PATCH请求路由，并允许传递额外的参数
    template<typename... AP>
    void PATCH(const std::string &route, const SeriesHandler &handler, const AP &... ap);
    // 模板函数，用于注册支持异步序列化任务的PATCH请求路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void PATCH(const std::string &route, int compute_queue_id,
            const SeriesHandler &handler, const AP &... ap);

    // 模板函数，用于注册支持异步序列化任务的PUT请求路由，并允许传递额外的参数
    template<typename... AP>
    void PUT(const std::string &route, const SeriesHandler &handler, const AP &... ap);
    // 模板函数，用于注册支持异步序列化任务的PUT请求路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void PUT(const std::string &route, int compute_queue_id,
            const SeriesHandler &handler, const AP &... ap);

    // 模板函数，用于注册支持异步序列化任务的HEAD请求路由，并允许传递额外的参数
    template<typename... AP>
    void HEAD(const std::string &route, const SeriesHandler &handler, const AP &... ap);
    // 模板函数，用于注册支持异步序列化任务的HEAD请求路由，并指定计算队列ID，同时允许传递额外的参数
    template<typename... AP>
    void HEAD(const std::string &route, int compute_queue_id,
            const SeriesHandler &handler, const AP &... ap);

public:
    // 获取内部路由对象的引用
    const Router &router() const
    { return router_; }

    // 将一个 BluePrint 对象添加到当前 BluePrint 中，并指定 URL 前缀
    void add_blueprint(const BluePrint &bp, const std::string &url_prefix);

    // 打印路由树的结构信息，用于测试和调试
    void print_node_arch() { router_.print_node_arch(); }  // for test

private:
    // 内部路由对象，用于存储和管理路由
    Router router_;    // ptr for hiding internal class

    // 声明 HttpServer 为友元类，允许 HttpServer 访问 BluePrint 的私有成员
    friend class HttpServer;
};


} // namespace Yukino

#include "BluePrint.inl"

#endif // YUKINO_BLUEPRINT_H_