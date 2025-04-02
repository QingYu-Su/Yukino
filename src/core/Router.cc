#include "workflow/HttpUtil.h"

#include "Router.h"
#include "HttpServerTask.h"
#include "HttpMsg.h"
#include "ErrorCode.h"
#include "CodeUtil.h"

using namespace Yukino;

// 注册路由和对应的处理函数
void Router::handle(const std::string &route, int compute_queue_id, const WrapHandler &handler, Verb verb)
{
    // 添加路由和HTTP请求方法的映射
    std::pair<RouteVerbIter, bool> rv_pair = add_route(verb, route);
    // 在路由表中查找或创建路由对应的VerbHandler
    VerbHandler &vh = routes_map_.find_or_create(rv_pair.first->route.c_str());
    // 检查是否已经存在相同的HTTP请求方法
    if(vh.verb_handler_map.find(verb) != vh.verb_handler_map.end())
    {
        fprintf(stderr, "Duplicate Verb\n");
        return;
    }

    // 将处理函数插入到VerbHandler的映射中
    vh.verb_handler_map.insert({verb, handler});
    vh.path = rv_pair.first->route;
    vh.compute_queue_id = compute_queue_id;
}

// 调用路由对应的处理函数
int Router::call(Verb verb, const std::string &route, HttpServerTask *server_task) const
{
    HttpReq *req = server_task->get_req();  // 获取请求对象
    HttpResp *resp = server_task->get_resp();  // 获取响应对象

    // 去掉路由路径末尾的"/"，除非路径是"/"
    StringPiece route2(route);
    if (route2.size() > 1 and route2[static_cast<int>(route2.size()) - 1] == '/')
        route2.remove_suffix(1);

    std::map<std::string, std::string> route_params;  // 用于存储路由参数
    std::string route_match_path;  // 匹配的路由路径
    // 在路由表中查找匹配的路由
    auto it = routes_map_.find(route2, route_params, route_match_path);

    int error_code = StatusOK;  // 默认状态码为成功
    if (it != routes_map_.end())   // 找到匹配的路由
    {
        // 检查是否存在对应的HTTP请求方法
        std::map<Verb, WrapHandler> &verb_handler_map = it->second.verb_handler_map;
        bool has_verb = verb_handler_map.find(verb) != verb_handler_map.end() ? true : false;
        if(verb_handler_map.find(Verb::ANY) != verb_handler_map.end() or has_verb)
        {
            // 设置请求的完整路径、路由参数和匹配路径
            req->set_full_path(it->second.path.as_string());
            req->set_route_params(std::move(route_params));
            req->set_route_match_path(std::move(route_match_path));
            WFGoTask * go_task;
            if(has_verb)
            {
                // 如果找到具体的HTTP请求方法，调用对应的处理函数
                go_task = it->second.verb_handler_map[verb](req, resp, series_of(server_task));
            } else
            {
                // 如果找到通用的ANY方法，调用对应的处理函数
                go_task = it->second.verb_handler_map[Verb::ANY](req, resp, series_of(server_task));
            }
            if(go_task)
                **server_task << go_task;  // 将任务加入到任务队列中
        } else
        {
            error_code = StatusRouteVerbNotImplment;  // 未实现的HTTP请求方法
        }
    } else
    {
        error_code = StatusRouteNotFound;  // 路由未找到
    }
    return error_code;
}

// 打印路由信息
void Router::print_routes() const
{
    // 遍历所有路由
    for(auto &rv : routes_)
    {
        // 遍历每个路由支持的HTTP请求方法
        for(auto verb : rv.verbs)
        {
            if (CodeUtil::is_url_encode(rv.route))
            {
                // 如果路由路径是URL编码的，解码后打印
                fprintf(stderr, "[YUKINO] %s\t%s\n", verb_to_str(verb), CodeUtil::url_decode(rv.route).c_str());
            } else
            {
                // 直接打印路由路径
                fprintf(stderr, "[YUKINO] %s\t%s\n", verb_to_str(verb), rv.route.c_str());
            }
        }
    }
}

// 获取所有路由及其对应的HTTP请求方法
std::vector<std::pair<std::string, std::string>> Router::all_routes() const
{
    std::vector<std::pair<std::string, std::string>> res; // 用于存储结果
    // 遍历路由表，收集所有路由及其对应的HTTP请求方法
    routes_map_.all_routes([&res](const std::string &prefix, const VerbHandler &verb_handler)
    {
        // 遍历每个路由对应的HTTP请求方法映射
        for(auto& vh : verb_handler.verb_handler_map)
        {
            // 将HTTP请求方法和路由路径作为一对字符串存入结果向量
            res.emplace_back(verb_to_str(vh.first), prefix.c_str());
        }
    });
    return res; // 返回所有路由及其对应的HTTP请求方法
}

// 添加单个HTTP请求方法和路由的映射
std::pair<Router::RouteVerbIter, bool> Router::add_route(Verb verb, const std::string &route)
{
    RouteVerb rv; // 创建一个RouteVerb对象
    rv.route = route; // 设置路由路径
    // 在路由集合中查找是否存在相同的路由
    auto it = routes_.find(rv);
    if(it != routes_.end())
    {
        // 如果找到相同的路由，将新的HTTP请求方法添加到该路由的集合中
        it->verbs.insert(verb);
    } 
    else
    {
        // 如果未找到相同的路由，将新的HTTP请求方法添加到新创建的RouteVerb对象的集合中
        rv.verbs.insert(verb);
    }
    // 将新创建的RouteVerb对象或更新后的RouteVerb对象插入到路由集合中
    return routes_.emplace(std::move(rv));
}

// 添加多个HTTP请求方法和路由的映射
std::pair<Router::RouteVerbIter, bool> Router::add_route(const std::vector<Verb> &verbs, const std::string &route)
{
    RouteVerb rv; // 创建一个RouteVerb对象
    rv.route = route; // 设置路由路径
    // 在路由集合中查找是否存在相同的路由
    auto it = routes_.find(rv);
    if(it != routes_.end())
    {
        // 如果找到相同的路由，将多个新的HTTP请求方法逐一添加到该路由的集合中
        for(auto verb : verbs)
            it->verbs.insert(verb);
    } 
    else
    {
        // 如果未找到相同的路由，将多个新的HTTP请求方法逐一添加到新创建的RouteVerb对象的集合中
        for(auto verb : verbs)
            rv.verbs.insert(verb);
    }
    // 将新创建的RouteVerb对象或更新后的RouteVerb对象插入到路由集合中
    return routes_.emplace(std::move(rv));
}