#include "workflow/WFTaskFactory.h"

namespace Yukino
{

namespace detail
{

// 为了减少泛型编程的使用，添加了一些冗余代码
template<typename Tuple>
WFGoTask *aop_process(const Handler &handler,
                      const HttpReq *req,
                      HttpResp *resp,
                      Tuple *tp)
{
    // 在处理请求之前，调用 aop_before 函数，执行前置切面逻辑
    bool ret = aop_before(req, resp, *tp);
    if (!ret)
    {
        // 如果前置切面逻辑返回 false，则直接返回 nullptr，表示请求被拦截
        return nullptr;
    }

    // 获取全局切面实例
    GlobalAspect *global_aspect = GlobalAspect::get_instance();
    // 遍历全局切面列表，执行每个切面的前置逻辑
    for(auto asp : global_aspect->aspect_list)
    {
        ret = asp->before(req, resp);
        if(!ret) return nullptr; // 如果任意一个切面的前置逻辑返回 false，则拦截请求
    }

    // 调用实际的请求处理函数
    handler(req, resp);

    // 获取当前请求对应的 HttpServerTask 对象
    HttpServerTask *server_task = task_of(resp);

    // 为任务添加回调函数，用于执行后置切面逻辑
    server_task->add_callback([req, resp, tp, global_aspect](HttpTask *) 
    {
        // 执行 aop_after 函数，执行后置切面逻辑
        aop_after(req, resp, *tp);

        // 遍历全局切面列表（逆序），执行每个切面的后置逻辑
        for(auto it = global_aspect->aspect_list.rbegin(); it != global_aspect->aspect_list.rend(); ++it)
        {
            (*it)->after(req, resp);
        }

        // 释放 Tuple 对象
        delete tp;
    });

    // 返回 nullptr，表示请求处理完成
    return nullptr;
}

// 与上述函数类似，但支持 SeriesHandler 类型的处理函数
template<typename Tuple>
WFGoTask *aop_process(const SeriesHandler &handler,
                      const HttpReq *req,
                      HttpResp *resp,
                      SeriesWork *series,
                      Tuple *tp)
{
    // 在处理请求之前，调用 aop_before 函数，执行前置切面逻辑
    bool ret = aop_before(req, resp, *tp);
    if (!ret)
    {
        // 如果前置切面逻辑返回 false，则直接返回 nullptr，表示请求被拦截
        return nullptr;
    }

    // 获取全局切面实例
    GlobalAspect *global_aspect = GlobalAspect::get_instance();
    // 遍历全局切面列表，执行每个切面的前置逻辑
    for(auto asp : global_aspect->aspect_list)
    {
        ret = asp->before(req, resp);
        if(!ret) return nullptr; // 如果任意一个切面的前置逻辑返回 false，则拦截请求
    }

    // 调用实际的请求处理函数（支持异步序列化任务）
    handler(req, resp, series);

    // 获取当前请求对应的 HttpServerTask 对象
    HttpServerTask *server_task = task_of(resp);

    // 为任务添加回调函数，用于执行后置切面逻辑
    server_task->add_callback([req, resp, tp, global_aspect](HttpTask *) 
    {
        // 执行 aop_after 函数，执行后置切面逻辑
        aop_after(req, resp, *tp);

        // 遍历全局切面列表（逆序），执行每个切面的后置逻辑
        for(auto it = global_aspect->aspect_list.rbegin(); it != global_aspect->aspect_list.rend(); ++it)
        {
            (*it)->after(req, resp);
        }

        // 释放 Tuple 对象
        delete tp;
    });

    // 返回 nullptr，表示请求处理完成
    return nullptr;
}

template<typename Tuple>
WFGoTask *aop_compute_process(const Handler &handler,
                              int compute_queue_id,
                              const HttpReq *req,
                              HttpResp *resp,
                              Tuple *tp)
{
    // 在处理请求之前，调用 aop_before 函数，执行前置切面逻辑
    bool ret = aop_before(req, resp, *tp);
    if (!ret)
    {
        // 如果前置切面逻辑返回 false，则直接返回 nullptr，表示请求被拦截
        return nullptr;
    }

    // 获取全局切面实例
    GlobalAspect *global_aspect = GlobalAspect::get_instance();
    // 遍历全局切面列表，执行每个切面的前置逻辑
    for(auto asp : global_aspect->aspect_list)
    {
        ret = asp->before(req, resp);
        if(!ret) return nullptr; // 如果任意一个切面的前置逻辑返回 false，则拦截请求
    }

    // 创建一个 WFGoTask 任务，指定计算队列ID
    WFGoTask *go_task = WFTaskFactory::create_go_task(
            "Yukino" + std::to_string(compute_queue_id), // 任务名称，包含计算队列ID
            handler, // 请求处理函数
            req, // HTTP请求对象
            resp); // HTTP响应对象

    // 获取当前请求对应的 HttpServerTask 对象
    HttpServerTask *server_task = task_of(resp);

    // 为任务添加回调函数，用于执行后置切面逻辑
    server_task->add_callback([req, resp, tp, global_aspect](HttpTask *) 
    {
        // 执行 aop_after 函数，执行后置切面逻辑
        aop_after(req, resp, *tp);

        // 遍历全局切面列表（逆序），执行每个切面的后置逻辑
        for(auto it = global_aspect->aspect_list.rbegin(); it != global_aspect->aspect_list.rend(); ++it)
        {
            (*it)->after(req, resp);
        }

        // 释放 Tuple 对象，避免内存泄漏
        delete tp;
    });

    // 返回创建的 WFGoTask 任务
    return go_task;
}

template<typename Tuple>
WFGoTask *aop_compute_process(const SeriesHandler &handler,
                              int compute_queue_id,
                              const HttpReq *req,
                              HttpResp *resp,
                              SeriesWork *series,
                              Tuple *tp)
{
    // 在处理请求之前，调用 aop_before 函数，执行前置切面逻辑
    bool ret = aop_before(req, resp, *tp);
    if (!ret)
    {
        // 如果前置切面逻辑返回 false，则直接返回 nullptr，表示请求被拦截
        return nullptr;
    }

    // 获取全局切面实例
    GlobalAspect *global_aspect = GlobalAspect::get_instance();
    // 遍历全局切面列表，执行每个切面的前置逻辑
    for(auto asp : global_aspect->aspect_list)
    {
        ret = asp->before(req, resp);
        if(!ret) return nullptr; // 如果任意一个切面的前置逻辑返回 false，则拦截请求
    }

    // 创建一个 WFGoTask 任务，指定计算队列ID
    WFGoTask *go_task = WFTaskFactory::create_go_task(
            "Yukino" + std::to_string(compute_queue_id), // 任务名称，包含计算队列ID
            handler, // 请求处理函数
            req, // HTTP请求对象
            resp, // HTTP响应对象
            series); // SeriesWork 对象，用于支持异步序列化任务

    // 获取当前请求对应的 HttpServerTask 对象
    HttpServerTask *server_task = task_of(resp);

    // 为任务添加回调函数，用于执行后置切面逻辑
    server_task->add_callback([req, resp, tp, global_aspect](HttpTask *) 
    {
        // 执行 aop_after 函数，执行后置切面逻辑
        aop_after(req, resp, *tp);

        // 遍历全局切面列表（逆序），执行每个切面的后置逻辑
        for(auto it = global_aspect->aspect_list.rbegin(); it != global_aspect->aspect_list.rend(); ++it)
        {
            (*it)->after(req, resp);
        }

        // 释放 Tuple 对象，避免内存泄漏
        delete tp;
    });

    // 返回创建的 WFGoTask 任务
    return go_task;
}

}  // namespace detail

// 模板函数，用于注册路由，支持单一HTTP方法，并允许传递额外的参数（如切面、中间件等）
template<typename... AP>
void BluePrint::ROUTE(const std::string &route, const Handler &handler, 
                      Verb verb, const AP &... ap)
{
    // 创建一个包装处理函数，用于在实际处理函数之前和之后插入AOP逻辑
    WrapHandler wrap_handler =
            [handler, this, ap...](const HttpReq *req,
                                   HttpResp *resp,
                                   SeriesWork *) -> WFGoTask *
            {
                // 创建一个包含可变参数的元组
                auto *tp = new std::tuple<AP...>(std::move(ap)...);
                // 调用 aop_process 函数，执行AOP逻辑
                WFGoTask *go_task = detail::aop_process(handler,
                                                        req,
                                                        resp,
                                                        tp);
                return go_task;
            };

    // 将路由和包装处理函数注册到内部的 Router 对象中
    router_.handle(route, -1, wrap_handler, verb);
}

// 模板函数，用于注册路由，支持单一HTTP方法，并指定计算队列ID，同时允许传递额外的参数
template<typename... AP>
void BluePrint::ROUTE(const std::string &route, int compute_queue_id, 
                      const Handler &handler, Verb verb, const AP &... ap)
{
    // 创建一个包装处理函数，用于在实际处理函数之前和之后插入AOP逻辑
    WrapHandler wrap_handler =
            [handler, compute_queue_id, this, ap...](HttpReq *req,
                                                     HttpResp *resp,
                                                     SeriesWork *) -> WFGoTask *
            {
                // 创建一个包含可变参数的元组
                auto *tp = new std::tuple<AP...>(std::move(ap)...);
                // 调用 aop_compute_process 函数，执行AOP逻辑，并指定计算队列ID
                WFGoTask *go_task = detail::aop_compute_process(handler,
                                                                compute_queue_id,
                                                                req,
                                                                resp,
                                                                tp);
                return go_task;
            };

    // 将路由和包装处理函数注册到内部的 Router 对象中
    router_.handle(route, compute_queue_id, wrap_handler, verb);
}

// 模板函数，用于注册路由，支持多个HTTP方法（通过方法名字符串列表指定），并允许传递额外的参数
template<typename... AP>
void BluePrint::ROUTE(const std::string &route, const Handler &handler, 
                      const std::vector<std::string> &methods, const AP &... ap)
{
    // 遍历方法列表，为每个方法调用ROUTE函数
    for(const auto &method : methods)
    {
        this->ROUTE(route, handler, str_to_verb(method), ap...);
    }
}

// 模板函数，用于注册路由，支持多个HTTP方法，并指定计算队列ID，同时允许传递额外的参数
template<typename... AP>
void BluePrint::ROUTE(const std::string &route, int compute_queue_id, 
                      const Handler &handler, 
                      const std::vector<std::string> &methods, const AP &... ap)
{
    // 遍历方法列表，为每个方法调用ROUTE函数
    for(const auto &method : methods)
    {
        this->ROUTE(route, compute_queue_id, handler, str_to_verb(method), ap...);
    } 
}

// 模板函数，用于注册GET请求的路由，并允许传递额外的参数
template<typename... AP>
void BluePrint::GET(const std::string &route, const Handler &handler, const AP &... ap)
{
    // 调用ROUTE函数，指定HTTP方法为GET
    this->ROUTE(route, handler, Verb::GET, ap...);
}

// 模板函数，用于注册GET请求的路由，并指定计算队列ID，同时允许传递额外的参数
template<typename... AP>
void BluePrint::GET(const std::string &route, int compute_queue_id,
                    const Handler &handler, const AP &... ap)
{
    // 调用ROUTE函数，指定HTTP方法为GET
    this->ROUTE(route, compute_queue_id, handler, Verb::GET, ap...);
}

// 模板函数，用于注册POST请求的路由，并允许传递额外的参数
template<typename... AP>
void BluePrint::POST(const std::string &route, const Handler &handler, const AP &... ap)
{
    // 调用ROUTE函数，指定HTTP方法为POST
    this->ROUTE(route, handler, Verb::POST, ap...);
}

// 模板函数，用于注册POST请求的路由，并指定计算队列ID，同时允许传递额外的参数
template<typename... AP>
void BluePrint::POST(const std::string &route, int compute_queue_id,
                     const Handler &handler, const AP &... ap)
{
    // 调用ROUTE函数，指定HTTP方法为POST
    this->ROUTE(route, compute_queue_id, handler, Verb::POST, ap...);
}

// 模板函数，用于注册DELETE请求的路由，并允许传递额外的参数
template<typename... AP>
void BluePrint::DELETE(const std::string &route, const Handler &handler, const AP &... ap)
{
    // 调用ROUTE函数，指定HTTP方法为DELETE
    this->ROUTE(route, handler, Verb::DELETE, ap...);
}

// 模板函数，用于注册DELETE请求的路由，并指定计算队列ID，同时允许传递额外的参数
template<typename... AP>
void BluePrint::DELETE(const std::string &route, int compute_queue_id,
                       const Handler &handler, const AP &... ap)
{
    // 调用ROUTE函数，指定HTTP方法为DELETE
    this->ROUTE(route, compute_queue_id, handler, Verb::DELETE, ap...);
}

// 模板函数，用于注册PATCH请求的路由，并允许传递额外的参数
template<typename... AP>
void BluePrint::PATCH(const std::string &route, const Handler &handler, const AP &... ap)
{
    // 调用ROUTE函数，指定HTTP方法为PATCH
    this->ROUTE(route, handler, Verb::PATCH, ap...);
}

// 模板函数，用于注册PATCH请求的路由，并指定计算队列ID，同时允许传递额外的参数
template<typename... AP>
void BluePrint::PATCH(const std::string &route, int compute_queue_id,
                      const Handler &handler, const AP &... ap)
{
    // 调用ROUTE函数，指定HTTP方法为PATCH
    this->ROUTE(route, compute_queue_id, handler, Verb::PATCH, ap...);
}

// 模板函数，用于注册PUT请求的路由，并允许传递额外的参数
template<typename... AP>
void BluePrint::PUT(const std::string &route, const Handler &handler, const AP &... ap)
{
    // 调用ROUTE函数，指定HTTP方法为PUT
    this->ROUTE(route, handler, Verb::PUT, ap...);
}

// 模板函数，用于注册PUT请求的路由，并指定计算队列ID，同时允许传递额外的参数
template<typename... AP>
void BluePrint::PUT(const std::string &route, int compute_queue_id,
                    const Handler &handler, const AP &... ap)
{
    // 调用ROUTE函数，指定HTTP方法为PUT
    this->ROUTE(route, compute_queue_id, handler, Verb::PUT, ap...);
}

// 模板函数，用于注册HEAD请求的路由，并允许传递额外的参数
template<typename... AP>
void BluePrint::HEAD(const std::string &route, const Handler &handler, const AP &... ap)
{
    // 调用ROUTE函数，指定HTTP方法为HEAD
    this->ROUTE(route, handler, Verb::HEAD, ap...);
}

// 模板函数，用于注册HEAD请求的路由，并指定计算队列ID，同时允许传递额外的参数
template<typename... AP>
void BluePrint::HEAD(const std::string &route, int compute_queue_id,
                     const Handler &handler, const AP &... ap)
{
    // 调用ROUTE函数，指定HTTP方法为HEAD
    this->ROUTE(route, compute_queue_id, handler, Verb::HEAD, ap...);
}

// 模板函数，用于注册路由，支持单一HTTP方法，并允许传递额外的参数（如切面、中间件等）
// 这里支持 SeriesHandler 类型的处理函数，用于处理异步序列化任务
template<typename... AP>
void BluePrint::ROUTE(const std::string &route, const SeriesHandler &handler, 
                      Verb verb, const AP &... ap)
{
    // 创建一个包装处理函数，用于在实际处理函数之前和之后插入AOP逻辑
    WrapHandler wrap_handler =
            [handler, this, ap...](const HttpReq *req,
                                   HttpResp *resp,
                                   SeriesWork *series) -> WFGoTask *
            {
                // 创建一个包含可变参数的元组
                auto *tp = new std::tuple<AP...>(std::move(ap)...);
                // 调用 aop_process 函数，执行AOP逻辑
                WFGoTask *go_task = detail::aop_process(handler,
                                                        req,
                                                        resp,
                                                        series,
                                                        tp);
                return go_task;
            };

    // 将路由和包装处理函数注册到内部的 Router 对象中
    router_.handle(route, -1, wrap_handler, verb);
}

// 模板函数，用于注册路由，支持单一HTTP方法，并指定计算队列ID，同时允许传递额外的参数
// 这里支持 SeriesHandler 类型的处理函数，用于处理异步序列化任务
template<typename... AP>
void BluePrint::ROUTE(const std::string &route, int compute_queue_id, 
                      const SeriesHandler &handler, Verb verb, const AP &... ap)
{
    // 创建一个包装处理函数，用于在实际处理函数之前和之后插入AOP逻辑
    WrapHandler wrap_handler =
            [handler, compute_queue_id, this, ap...](HttpReq *req,
                                                     HttpResp *resp,
                                                     SeriesWork *series) -> WFGoTask *
            {
                // 创建一个包含可变参数的元组
                auto *tp = new std::tuple<AP...>(std::move(ap)...);
                // 调用 aop_compute_process 函数，执行AOP逻辑，并指定计算队列ID
                WFGoTask *go_task = detail::aop_compute_process(handler,
                                                                compute_queue_id,
                                                                req,
                                                                resp,
                                                                series,
                                                                tp);
                return go_task;
            };

    // 将路由和包装处理函数注册到内部的 Router 对象中
    router_.handle(route, compute_queue_id, wrap_handler, verb);  
}

// 模板函数，用于注册路由，支持多个HTTP方法（通过方法名字符串列表指定），并允许传递额外的参数
template<typename... AP>
void BluePrint::ROUTE(const std::string &route, const SeriesHandler &handler, 
                      const std::vector<std::string> &methods, const AP &... ap)
{
    // 遍历方法列表，为每个方法调用ROUTE函数
    for(const auto &method : methods)
    {
        this->ROUTE(route, handler, str_to_verb(method), ap...);
    } 
}

// 模板函数，用于注册路由，支持多个HTTP方法，并指定计算队列ID，同时允许传递额外的参数
template<typename... AP>
void BluePrint::ROUTE(const std::string &route, int compute_queue_id, 
                      const SeriesHandler &handler, 
                      const std::vector<std::string> &methods, const AP &... ap)
{
    // 遍历方法列表，为每个方法调用ROUTE函数
    for(const auto &method : methods)
    {
        this->ROUTE(route, compute_queue_id, handler, str_to_verb(method), ap...);
    } 
}

// 模板函数，用于注册GET请求的路由，并允许传递额外的参数
template<typename... AP>
void BluePrint::GET(const std::string &route, const SeriesHandler &handler, const AP &... ap)
{
    // 调用ROUTE函数，指定HTTP方法为GET
    this->ROUTE(route, handler, Verb::GET, ap...);
}

// 模板函数，用于注册GET请求的路由，并指定计算队列ID，同时允许传递额外的参数
template<typename... AP>
void BluePrint::GET(const std::string &route, int compute_queue_id,
                    const SeriesHandler &handler, const AP &... ap)
{   
    // 调用ROUTE函数，指定HTTP方法为GET
    this->ROUTE(route, compute_queue_id, handler, Verb::GET, ap...);
}

// 模板函数，用于注册POST请求的路由，并允许传递额外的参数
template<typename... AP>
void BluePrint::POST(const std::string &route, const SeriesHandler &handler, const AP &... ap)
{
    // 调用ROUTE函数，指定HTTP方法为POST
    this->ROUTE(route, handler, Verb::POST, ap...);
}

// 模板函数，用于注册POST请求的路由，并指定计算队列ID，同时允许传递额外的参数
template<typename... AP>
void BluePrint::POST(const std::string &route, int compute_queue_id,
                     const SeriesHandler &handler, const AP &... ap)
{
    // 调用ROUTE函数，指定HTTP方法为POST
    this->ROUTE(route, compute_queue_id, handler, Verb::POST, ap...);
}

// 模板函数，用于注册DELETE请求的路由，并允许传递额外的参数
template<typename... AP>
void BluePrint::DELETE(const std::string &route, const SeriesHandler &handler, const AP &... ap)
{
    // 调用ROUTE函数，指定HTTP方法为DELETE
    this->ROUTE(route, handler, Verb::DELETE, ap...);
}

// 模板函数，用于注册DELETE请求的路由，并指定计算队列ID，同时允许传递额外的参数
template<typename... AP>
void BluePrint::DELETE(const std::string &route, int compute_queue_id,
                    const SeriesHandler &handler, const AP &... ap)
{   
    // 调用ROUTE函数，指定HTTP方法为DELETE
    this->ROUTE(route, compute_queue_id, handler, Verb::DELETE, ap...);
}

// 模板函数，用于注册PATCH请求的路由，并允许传递额外的参数
template<typename... AP>
void BluePrint::PATCH(const std::string &route, const SeriesHandler &handler, const AP &... ap)
{
    // 调用ROUTE函数，指定HTTP方法为PATCH
    this->ROUTE(route, handler, Verb::PATCH, ap...);
}

// 模板函数，用于注册PATCH请求的路由，并指定计算队列ID，同时允许传递额外的参数
template<typename... AP>
void BluePrint::PATCH(const std::string &route, int compute_queue_id,
                    const SeriesHandler &handler, const AP &... ap)
{   
    // 调用ROUTE函数，指定HTTP方法为PATCH
    this->ROUTE(route, compute_queue_id, handler, Verb::PATCH, ap...);
}

// 模板函数，用于注册PUT请求的路由，并允许传递额外的参数
template<typename... AP>
void BluePrint::PUT(const std::string &route, const SeriesHandler &handler, const AP &... ap)
{
    // 调用ROUTE函数，指定HTTP方法为PUT
    this->ROUTE(route, handler, Verb::PUT, ap...);
}

// 模板函数，用于注册PUT请求的路由，并指定计算队列ID，同时允许传递额外的参数
template<typename... AP>
void BluePrint::PUT(const std::string &route, int compute_queue_id,
                    const SeriesHandler &handler, const AP &... ap)
{   
    // 调用通用的ROUTE函数，指定HTTP方法为PUT
    this->ROUTE(route, compute_queue_id, handler, Verb::PUT, ap...);
}

// 模板函数，用于注册HEAD请求的路由，并允许传递额外的参数
template<typename... AP>
void BluePrint::HEAD(const std::string &route, const SeriesHandler &handler, const AP &... ap)
{
    // 调用通用的ROUTE函数，指定HTTP方法为HEAD
    this->ROUTE(route, handler, Verb::HEAD, ap...);
}

// 模板函数，用于注册HEAD请求的路由，并指定计算队列ID，同时允许传递额外的参数
template<typename... AP>
void BluePrint::HEAD(const std::string &route, int compute_queue_id,
                    const SeriesHandler &handler, const AP &... ap)
{   
    // 调用通用的ROUTE函数，指定HTTP方法为HEAD
    this->ROUTE(route, compute_queue_id, handler, Verb::HEAD, ap...);
}

} // namespace Yukino





