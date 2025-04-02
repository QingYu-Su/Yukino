#include "BluePrint.h"
#include "HttpMsg.h"

using namespace Yukino;

// 注册路由，支持单一HTTP方法，不指定计算队列ID
void BluePrint::ROUTE(const std::string &route, const Handler &handler, Verb verb)
{
    // 创建一个包装处理函数，用于在实际处理函数之前和之后插入AOP逻辑
    WrapHandler wrap_handler =
            [handler](const HttpReq *req,
                      HttpResp *resp,
                      SeriesWork *) -> WFGoTask *
            {
                // 获取全局切面实例
                GlobalAspect *global_aspect = GlobalAspect::get_instance();
                // 遍历全局切面列表，执行每个切面的前置逻辑
                for(auto asp : global_aspect->aspect_list)
                {
                    auto ret = asp->before(req, resp);
                    if(!ret) return nullptr; // 如果任意一个切面的前置逻辑返回 false，则拦截请求
                }

                // 调用实际的请求处理函数
                handler(req, resp);

                // 如果存在全局切面逻辑，为任务添加回调函数，用于执行后置逻辑
                if(!global_aspect->aspect_list.empty())
                {
                    HttpServerTask *server_task = task_of(resp);
                    server_task->add_callback([req, resp, global_aspect](HttpTask *)
                    {
                        // 遍历全局切面列表（逆序），执行每个切面的后置逻辑
                        for(auto it = global_aspect->aspect_list.rbegin(); it != global_aspect->aspect_list.rend(); ++it)
                        {
                            (*it)->after(req, resp);
                        }
                    });
                }
                return nullptr; // 返回 nullptr，表示请求处理完成
            };

    // 将路由和包装处理函数注册到内部的 Router 对象中
    router_.handle(route, -1, wrap_handler, verb);
}

// 注册路由，支持单一HTTP方法，并指定计算队列ID
void BluePrint::ROUTE(const std::string &route, int compute_queue_id, const Handler &handler, Verb verb)
{
    // 创建一个包装处理函数，用于在实际处理函数之前和之后插入AOP逻辑
    WrapHandler wrap_handler =
            [handler, compute_queue_id](HttpReq *req,
                                        HttpResp *resp,
                                        SeriesWork *) -> WFGoTask *
            {
                // 获取全局切面实例
                GlobalAspect *global_aspect = GlobalAspect::get_instance();
                // 遍历全局切面列表，执行每个切面的前置逻辑
                for(auto asp : global_aspect->aspect_list)
                {
                    auto ret = asp->before(req, resp);
                    if(!ret) return nullptr; // 如果任意一个切面的前置逻辑返回 false，则拦截请求
                }

                // 创建一个 WFGoTask 任务，指定计算队列ID
                WFGoTask *go_task = WFTaskFactory::create_go_task(
                        "Yukino" + std::to_string(compute_queue_id),
                        handler,
                        req,
                        resp);

                // 如果存在全局切面逻辑，为任务添加回调函数，用于执行后置逻辑
                if(!global_aspect->aspect_list.empty())
                {
                    HttpServerTask *server_task = task_of(resp);
                    server_task->add_callback([req, resp, global_aspect](HttpTask *)
                    {
                        // 遍历全局切面列表（逆序），执行每个切面的后置逻辑
                        for(auto it = global_aspect->aspect_list.rbegin(); it != global_aspect->aspect_list.rend(); ++it)
                        {
                            (*it)->after(req, resp);
                        }
                    });
                }
                return go_task; // 返回创建的 WFGoTask 任务
            };

    // 将路由和包装处理函数注册到内部的 Router 对象中
    router_.handle(route, compute_queue_id, wrap_handler, verb);
}

// 注册路由，支持多个HTTP方法（通过方法名字符串列表指定）
void BluePrint::ROUTE(const std::string &route, const Handler &handler, const std::vector<std::string> &methods)
{
    // 遍历方法列表，为每个方法调用ROUTE函数
    for(const auto &method : methods)
    {
        this->ROUTE(route, handler, str_to_verb(method));
    }
}

// 注册路由，支持多个HTTP方法，并指定计算队列ID
void BluePrint::ROUTE(const std::string &route, int compute_queue_id, const Handler &handler, const std::vector<std::string> &methods)
{
    // 遍历方法列表，为每个方法调用ROUTE函数
    for(const auto &method : methods)
    {
        this->ROUTE(route, compute_queue_id, handler, str_to_verb(method));
    }
}

// 注册GET请求的路由
void BluePrint::GET(const std::string &route, const Handler &handler)
{
    // 调用ROUTE函数，指定HTTP方法为GET
    this->ROUTE(route, handler, Verb::GET);
}

// 注册GET请求的路由，并指定计算队列ID
void BluePrint::GET(const std::string &route, int compute_queue_id, const Handler &handler)
{
    // 调用ROUTE函数，指定HTTP方法为GET
    this->ROUTE(route, compute_queue_id, handler, Verb::GET);
}

// 注册POST请求的路由
void BluePrint::POST(const std::string &route, const Handler &handler)
{
    // 调用ROUTE函数，指定HTTP方法为POST
    this->ROUTE(route, handler, Verb::POST);
}

// 注册POST请求的路由，并指定计算队列ID
void BluePrint::POST(const std::string &route, int compute_queue_id, const Handler &handler)
{
    // 调用ROUTE函数，指定HTTP方法为POST
    this->ROUTE(route, compute_queue_id, handler, Verb::POST);
}

// 注册DELETE请求的路由
void BluePrint::DELETE(const std::string &route, const Handler &handler)
{
    // 调用ROUTE函数，指定HTTP方法为DELETE
    this->ROUTE(route, handler, Verb::DELETE);
}

// 注册DELETE请求的路由，并指定计算队列ID
void BluePrint::DELETE(const std::string &route, int compute_queue_id, const Handler &handler)
{
    // 调用ROUTE函数，指定HTTP方法为DELETE
    this->ROUTE(route, compute_queue_id, handler, Verb::DELETE);
}

// 注册PATCH请求的路由
void BluePrint::PATCH(const std::string &route, const Handler &handler)
{
    // 调用ROUTE函数，指定HTTP方法为PATCH
    this->ROUTE(route, handler, Verb::PATCH);
}

// 注册PATCH请求的路由，并指定计算队列ID
void BluePrint::PATCH(const std::string &route, int compute_queue_id, const Handler &handler)
{
    // 调用ROUTE函数，指定HTTP方法为PATCH
    this->ROUTE(route, compute_queue_id, handler, Verb::PATCH);
}

// 注册PUT请求的路由
void BluePrint::PUT(const std::string &route, const Handler &handler)
{
    // 调用ROUTE函数，指定HTTP方法为PUT
    this->ROUTE(route, handler, Verb::PUT);
}

// 注册PUT请求的路由，并指定计算队列ID
void BluePrint::PUT(const std::string &route, int compute_queue_id, const Handler &handler)
{
    // 调用ROUTE函数，指定HTTP方法为PUT
    this->ROUTE(route, compute_queue_id, handler, Verb::PUT);
}

// 注册HEAD请求的路由
void BluePrint::HEAD(const std::string &route, const Handler &handler)
{
    // 调用ROUTE函数，指定HTTP方法为HEAD
    this->ROUTE(route, handler, Verb::HEAD);
}

// 注册HEAD请求的路由，并指定计算队列ID
void BluePrint::HEAD(const std::string &route, int compute_queue_id, const Handler &handler)
{
    // 调用ROUTE函数，指定HTTP方法为HEAD
    this->ROUTE(route, compute_queue_id, handler, Verb::HEAD);
}

// 注册路由，支持单一HTTP方法，不指定计算队列ID
void BluePrint::ROUTE(const std::string &route, const SeriesHandler &handler, Verb verb)
{
    // 创建一个包装处理函数，用于在实际处理函数之前和之后插入AOP逻辑
    WrapHandler wrap_handler =
            [handler](const HttpReq *req,
                      HttpResp *resp,
                      SeriesWork *series) -> WFGoTask *
            {
                // 获取全局切面实例
                GlobalAspect *global_aspect = GlobalAspect::get_instance();
                // 遍历全局切面列表，执行每个切面的前置逻辑
                for(auto asp : global_aspect->aspect_list)
                {
                    auto ret = asp->before(req, resp);
                    if(!ret) return nullptr; // 如果任意一个切面的前置逻辑返回 false，则拦截请求
                }

                // 调用实际的请求处理函数
                handler(req, resp, series);

                // 如果存在全局切面逻辑，为任务添加回调函数，用于执行后置逻辑
                if(!global_aspect->aspect_list.empty())
                {
                    HttpServerTask *server_task = task_of(resp);
                    server_task->add_callback([req, resp, global_aspect](HttpTask *)
                    {
                        // 遍历全局切面列表（逆序），执行每个切面的后置逻辑
                        for(auto it = global_aspect->aspect_list.rbegin(); it != global_aspect->aspect_list.rend(); ++it)
                        {
                            (*it)->after(req, resp);
                        }
                    });
                }
                return nullptr; // 返回 nullptr，表示请求处理完成
            };

    // 将路由和包装处理函数注册到内部的 Router 对象中
    router_.handle(route, -1, wrap_handler, verb);
}

// 注册路由，支持单一HTTP方法，并指定计算队列ID
void BluePrint::ROUTE(const std::string &route, int compute_queue_id, const SeriesHandler &handler, Verb verb)
{
    // 创建一个包装处理函数，用于在实际处理函数之前和之后插入AOP逻辑
    WrapHandler wrap_handler =
            [handler, compute_queue_id](HttpReq *req,
                                        HttpResp *resp,
                                        SeriesWork *series) -> WFGoTask *
            {
                // 获取全局切面实例
                GlobalAspect *global_aspect = GlobalAspect::get_instance();
                // 遍历全局切面列表，执行每个切面的前置逻辑
                for(auto asp : global_aspect->aspect_list)
                {
                    auto ret = asp->before(req, resp);
                    if(!ret) return nullptr; // 如果任意一个切面的前置逻辑返回 false，则拦截请求
                }

                // 创建一个 WFGoTask 任务，指定计算队列ID
                WFGoTask *go_task = WFTaskFactory::create_go_task(
                        "Yukino" + std::to_string(compute_queue_id),
                        handler,
                        req,
                        resp,
                        series);

                // 如果存在全局切面逻辑，为任务添加回调函数，用于执行后置逻辑
                if(!global_aspect->aspect_list.empty())
                {
                    HttpServerTask *server_task = task_of(resp);
                    server_task->add_callback([req, resp, global_aspect](HttpTask *)
                    {
                        // 遍历全局切面列表（逆序），执行每个切面的后置逻辑
                        for(auto it = global_aspect->aspect_list.rbegin(); it != global_aspect->aspect_list.rend(); ++it)
                        {
                            (*it)->after(req, resp);
                        }
                    });
                }
                return go_task; // 返回创建的 WFGoTask 任务
            };

    // 将路由和包装处理函数注册到内部的 Router 对象中
    router_.handle(route, compute_queue_id, wrap_handler, verb);
}

// 注册路由，支持多个HTTP方法（通过方法名字符串列表指定）
void BluePrint::ROUTE(const std::string &route, const SeriesHandler &handler, const std::vector<std::string> &methods)
{
    // 遍历方法列表，为每个方法调用ROUTE函数
    for(const auto &method : methods)
    {
        this->ROUTE(route, handler, str_to_verb(method));
    }
}

// 注册路由，支持多个HTTP方法，并指定计算队列ID
void BluePrint::ROUTE(const std::string &route, int compute_queue_id,
            const SeriesHandler &handler, const std::vector<std::string> &methods)
{
    // 遍历方法列表，为每个方法调用ROUTE函数
    for(const auto &method : methods)
    {
        this->ROUTE(route, compute_queue_id, handler, str_to_verb(method));
    }
}

// 注册GET请求的路由
void BluePrint::GET(const std::string &route, const SeriesHandler &handler)
{
    // 调用ROUTE函数，指定HTTP方法为GET
    this->ROUTE(route, -1, handler, Verb::GET);
}

// 注册GET请求的路由，并指定计算队列ID
void BluePrint::GET(const std::string &route, int compute_queue_id, const SeriesHandler &handler)
{
    // 调用ROUTE函数，指定HTTP方法为GET
    this->ROUTE(route, compute_queue_id, handler, Verb::GET);
}

// 注册 POST 请求的路由
void BluePrint::POST(const std::string &route, const SeriesHandler &handler)
{
    // 调用通用的 ROUTE 方法，指定 HTTP 方法为 POST
    this->ROUTE(route, -1, handler, Verb::POST);
}

// 注册 POST 请求的路由，并指定计算队列 ID
void BluePrint::POST(const std::string &route, int compute_queue_id, const SeriesHandler &handler)
{
    // 调用通用的 ROUTE 方法，指定 HTTP 方法为 POST
    this->ROUTE(route, compute_queue_id, handler, Verb::POST);
}

// 注册 DELETE 请求的路由
void BluePrint::DELETE(const std::string &route, const SeriesHandler &handler)
{
    // 调用通用的 ROUTE 方法，指定 HTTP 方法为 DELETE
    this->ROUTE(route, -1, handler, Verb::DELETE);
}

// 注册 DELETE 请求的路由，并指定计算队列 ID
void BluePrint::DELETE(const std::string &route, int compute_queue_id, const SeriesHandler &handler)
{
    // 调用通用的 ROUTE 方法，指定 HTTP 方法为 DELETE
    this->ROUTE(route, compute_queue_id, handler, Verb::DELETE);
}

// 注册 PATCH 请求的路由
void BluePrint::PATCH(const std::string &route, const SeriesHandler &handler)
{
    // 调用通用的 ROUTE 方法，指定 HTTP 方法为 PATCH
    this->ROUTE(route, -1, handler, Verb::PATCH);
}

// 注册 PATCH 请求的路由，并指定计算队列 ID
void BluePrint::PATCH(const std::string &route, int compute_queue_id, const SeriesHandler &handler)
{
    // 调用通用的 ROUTE 方法，指定 HTTP 方法为 PATCH
    this->ROUTE(route, compute_queue_id, handler, Verb::PATCH);
}

// 注册 PUT 请求的路由
void BluePrint::PUT(const std::string &route, const SeriesHandler &handler)
{
    // 调用通用的 ROUTE 方法，指定 HTTP 方法为 PUT
    this->ROUTE(route, -1, handler, Verb::PUT);
}

// 注册 PUT 请求的路由，并指定计算队列 ID
void BluePrint::PUT(const std::string &route, int compute_queue_id, const SeriesHandler &handler)
{
    // 调用通用的 ROUTE 方法，指定 HTTP 方法为 PUT
    this->ROUTE(route, compute_queue_id, handler, Verb::PUT);
}

// 注册 HEAD 请求的路由
void BluePrint::HEAD(const std::string &route, const SeriesHandler &handler)
{
    // 调用通用的 ROUTE 方法，指定 HTTP 方法为 HEAD
    this->ROUTE(route, -1, handler, Verb::HEAD);
}

// 注册 HEAD 请求的路由，并指定计算队列 ID
void BluePrint::HEAD(const std::string &route, int compute_queue_id, const SeriesHandler &handler)
{
    // 调用通用的 ROUTE 方法，指定 HTTP 方法为 HEAD
    this->ROUTE(route, compute_queue_id, handler, Verb::HEAD);
}

// 将一个 BluePrint 的路由添加到当前 BluePrint 中，并指定 URL 前缀
void BluePrint::add_blueprint(const BluePrint &bp, const std::string &url_prefix)
{
    // 遍历传入 BluePrint 的所有路由
    bp.router_.routes_map_.all_routes([this, &url_prefix]
    (const std::string &sub_prefix, VerbHandler verb_handler)
    {
        // 构造完整的路由路径
        std::string path = url_prefix;
        if (!path.empty() && path.back() == '/')
        {
            path.pop_back(); // 去掉前缀末尾的 '/'
        }
        if (!sub_prefix.empty())
        {
            if (sub_prefix.front() != '/')
            {
                path += '/'; // 如果子前缀不是以 '/' 开头，则添加 '/'
            }
            path += sub_prefix; // 添加子前缀
        }
        if (!path.empty() && path.back() == '/')
        {
            path.pop_back(); // 去掉路径末尾的 '/'
        }
        if (path.empty())
        {
            path = "/"; // 如果路径为空，则设置为根路径 '/'
        }

        // 获取子路由支持的所有 HTTP 方法
        std::vector<Verb> verb_list;
        for(auto &vh_item : verb_handler.verb_handler_map)
        {
            verb_list.push_back(vh_item.first); // 将 HTTP 方法添加到列表
        }

        // 在当前 BluePrint 的路由表中添加路由
        std::pair<Router::RouteVerbIter, bool> rv_pair = this->router_.add_route(verb_list, path.c_str());

        // 获取或创建路由对应的 VerbHandler
        VerbHandler &vh = this->router_.routes_map_.find_or_create(rv_pair.first->route.c_str());
        vh = verb_handler; // 将子路由的处理函数赋值给当前路由
        vh.path = rv_pair.first->route; // 更新路由路径
    });
}

