#ifndef YUKINO_ROUTER_H_
#define YUKINO_ROUTER_H_

#include <functional>
#include "RouteTable.h"
#include "Noncopyable.h"

namespace Yukino
{

class HttpServerTask;

class Router : public Noncopyable
{
public:
    // 注册路由和对应的处理函数
    // route: 路由路径
    // compute_queue_id: 计算队列ID
    // handler: 路由处理函数
    // verb: HTTP请求方法（如GET、POST等）
    void handle(const std::string &route, int compute_queue_id, const WrapHandler &handler, Verb verb);

    // 调用路由对应的处理函数
    // verb: HTTP请求方法
    // route: 路由路径
    // server_task: HttpServerTask对象指针
    // 返回值: 路由匹配结果
    int call(Verb verb, const std::string &route, HttpServerTask *server_task) const;

    // 打印路由信息，用于日志记录
    void print_routes() const;

    // 获取所有路由信息，用于测试
    std::vector<std::pair<std::string, std::string>> all_routes() const;

    // 路由和HTTP请求方法的结构体
    struct RouteVerb
    {
        std::string route; // 路由路径
        mutable std::set<Verb> verbs; // 支持的HTTP请求方法集合
        // 自定义比较函数，用于在set中排序
        bool operator()(const RouteVerb& l, const RouteVerb& r) const
        {
            return l.route > r.route;
        }
    };

    // 路由集合的迭代器类型
    using RouteVerbIter = std::set<RouteVerb>::iterator;

    // 添加单个HTTP请求方法和路由的映射
    // verb: HTTP请求方法
    // route: 路由路径
    // 返回值: 插入结果（迭代器和是否插入成功）
    std::pair<RouteVerbIter, bool> add_route(Verb verb, const std::string &route);

    // 添加多个HTTP请求方法和路由的映射
    // verbs: HTTP请求方法集合
    // route: 路由路径
    // 返回值: 插入结果（迭代器和是否插入成功）
    std::pair<RouteVerbIter, bool> add_route(const std::vector<Verb> &verbs, const std::string &route);
    
    // 打印路由树的结构信息
    void print_node_arch() { routes_map_.print_node_arch(); }

private:
    RouteTable routes_map_; // 路由表，用于存储和匹配路由
    std::set<RouteVerb, RouteVerb> routes_;  // 存储路由和HTTP请求方法的集合
    friend class BluePrint; // 友元类，允许BluePrint访问Router的私有成员
};

}  // Yukino

#endif // YUKINO_ROUTER_H_