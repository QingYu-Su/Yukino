#ifndef YUKINO_ROUTETABLE_H_
#define YUKINO_ROUTETABLE_H_

#include <vector>
#include <memory>
#include <cassert>
#include <unordered_map>

#include "StringPiece.h"
#include "VerbHandler.h"

namespace Yukino
{

// 路由表节点类
class RouteTableNode : public Noncopyable
{
public:
    // 析构函数
    ~RouteTableNode();

    // 路由表节点的迭代器
    struct iterator
    {
        const RouteTableNode *ptr; // 指向当前节点
        StringPiece first;        // 路由路径（客户端访问的路径）
        VerbHandler second;       // 动词处理器

        iterator *operator->()
        { return this; }

        bool operator==(const iterator &other) const
        { return this->ptr == other.ptr; }

        bool operator!=(const iterator &other) const
        { return this->ptr != other.ptr; }
    };

    // 查找或创建路由处理器
    VerbHandler &find_or_create(const StringPiece &route, size_t cursor);

    // 获取迭代器的结束位置
    iterator end() const
    { return iterator{nullptr, StringPiece(), VerbHandler()}; }

    // 查找路由并返回迭代器
    iterator find(const StringPiece &route,
                  size_t cursor,
                  std::map<std::string, std::string> &route_params,
                  std::string &route_match_path) const;

    // 遍历所有路由（叶子节点）并调用回调函数
    // prefix为叶子节点的路径前缀（包括叶子节点的路径部分），通过递归不断叠加，最后通过func函数传递
    // 注意，除非是“/”节点，否则所有节点获得前缀，都是没有最前面的根路径的，得到的都是如"abc/def"
    template<typename Func>
    void all_routes(const Func &func, std::string prefix) const;

    // 打印节点结构（用于测试）
    void print_node_arch();  

private:
    VerbHandler verb_handler_; // 动词处理器
    std::map<StringPiece, RouteTableNode *> children_; // 子节点映射
};

// 遍历所有路由并调用回调函数
template<typename Func>
void RouteTableNode::all_routes(const Func &func, std::string prefix) const
{
    if (children_.empty())
    {
        func(prefix, verb_handler_);
    } 
    else
    {
        if (!prefix.empty() && prefix.back() != '/')
            prefix += '/';
        for (auto &pair: children_)
        {
            pair.second->all_routes(func, prefix + pair.first.as_string());
        }
    }
}

// 路由表类
class RouteTable : public Noncopyable
{ 
public:
    // 查找或创建路由处理器
    VerbHandler &find_or_create(const char *route);

    // 查找路由并返回迭代器
    RouteTableNode::iterator find(const StringPiece &route, 
                                  std::map<std::string, std::string> &route_params,
                                  std::string &route_match_path) const
    { return root_.find(route, 0, route_params, route_match_path); }

    // 遍历所有路由并调用回调函数
    template<typename Func>
    void all_routes(const Func &func) const
    { root_.all_routes(func, ""); }

    // 获取迭代器的结束位置
    RouteTableNode::iterator end() const
    { return root_.end(); }

    // 析构函数
    ~RouteTable() = default;

    // 打印节点结构（用于测试）
    void print_node_arch() { root_.print_node_arch(); }

private:
    RouteTableNode root_; // 根节点
    std::set<StringPiece> string_pieces_;  // 存储当前已注册的所有路径
};

} // namespace Yukino

#endif // YUKINO_ROUTETABLE_H_