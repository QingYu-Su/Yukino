#include <queue>
#include "RouteTable.h"
#include "spdlog/spdlog.h" 

using namespace Yukino;

// RouteTableNode 的析构函数
RouteTableNode::~RouteTableNode()
{
    // 遍历子节点映射，删除所有子节点
    for (auto &child: children_)
    {
        delete child.second;
    }
}

// 查找或创建路由处理器
VerbHandler &RouteTableNode::find_or_create(const StringPiece &route, size_t cursor)
{
    // 如果已经到达路由字符串的末尾，返回当前节点的动词处理器
    if (cursor == route.size())
        return verb_handler_;

    // 特殊情况：处理根路径 "/"
    if (cursor == 0 && route.as_string() == "/")
    {
        // 创建一个新的路由表节点
        auto *new_node = new RouteTableNode();
        // 将新节点插入到子节点映射中
        children_.insert({route, new_node});
        // 递归调用 find_or_create，继续处理下一个字符
        return new_node->find_or_create(route, ++cursor);
    }

    // 忽略路径末尾的 "/"
    if (cursor == route.size() - 1 && route[cursor] == '/')
    {
        // 如果当前字符是 "/" 且是最后一个字符，直接返回当前节点的动词处理器
        return verb_handler_;
    }

    // 跳过路径中的 "/"
    if (route[cursor] == '/')
        cursor++;

    // 找到下一个 "/" 的位置
    int anchor = cursor;
    while (cursor < route.size() && route[cursor] != '/')
        cursor++;

    // 提取路径中的中间部分（例如 "/{mid}/"）
    StringPiece mid(route.begin() + anchor, cursor - anchor);

    // 在子节点映射中查找中间部分
    auto it = children_.find(mid);
    if (it != children_.end())
    {
        // 如果找到匹配的子节点，递归调用 find_or_create，继续处理下一个字符
        return it->second->find_or_create(route, cursor);
    }
    else
    {
        // 如果没有找到匹配的子节点，创建一个新的路由表节点
        auto *new_node = new RouteTableNode();
        // 将新节点插入到子节点映射中
        children_.insert({mid, new_node});
        // 递归调用 find_or_create，继续处理下一个字符
        return new_node->find_or_create(route, cursor);
    }
}

RouteTableNode::iterator RouteTableNode::find(const StringPiece &route, size_t cursor,
                                              std::map<std::string, std::string> &route_params,
                                              std::string &route_match_path) const
{
    assert(cursor >= 0); // 确保游标位置有效

    // 如果已经到达路由字符串的末尾
    if (cursor == route.size())
    {
        // 如果当前节点有处理器或者没有子节点，则认为找到了匹配的路由
        if (!verb_handler_.verb_handler_map.empty() || children_.empty())
        {
            return iterator{this, route, verb_handler_}; // 返回当前节点的迭代器
        }
    }

    // 如果到达路由字符串的末尾，但有子节点且没有处理器
    if (cursor == route.size() && !children_.empty())
    {
        auto it = children_.find(StringPiece("*")); // 查找通配符子节点
        if (it != children_.end())
        {
            if (it->second->verb_handler_.verb_handler_map.empty())
                spdlog::error("[YUKINO] handler nullptr");
            return iterator{it->second, route, it->second->verb_handler_}; // 返回通配符子节点的迭代器
        }
    }

    // 如果到达路由字符串的末尾，且当前节点没有处理器
    if (cursor == route.size() && verb_handler_.verb_handler_map.empty())
        return iterator{nullptr, route, verb_handler_}; // 返回空迭代器

    // 处理根路径 "/"
    if (cursor == 0 && route.as_string() == "/")
    {
        auto it = children_.find(route); // 查找根路径子节点
        if (it != children_.end())
        {
            auto it2 = it->second->find(route, ++cursor, route_params, route_match_path); // 递归查找
            if (it2 != it->second->end())
                return it2; // 如果找到匹配的路由，返回迭代器
        }
    }

    if (route[cursor] == '/')
        cursor++; // 跳过路径中的 "/"

    int anchor = cursor; // 标记路径的起始位置
    while (cursor < route.size() && route[cursor] != '/')
        cursor++; // 找到下一个 "/"

    StringPiece mid(route.begin() + anchor, cursor - anchor); // 提取路径中间部分

    auto it = children_.find(mid); // 查找当前路径部分的子节点
    if (it != children_.end())
    {
        auto it2 = it->second->find(route, cursor, route_params, route_match_path); // 递归查找
        if (it2 != it->second->end())
            return it2; // 如果找到匹配的路由，返回迭代器
    }

    // 如果有子节点是路径参数（如 `{name}`）或通配符，选择它
    for (auto &kv : children_)
    {
        StringPiece param(kv.first);
        if (!param.empty() && param[param.size() - 1] == '*') // 如果是通配符参数
        {
            StringPiece match(param);
            match.remove_suffix(1); // 去掉通配符
            if (mid.starts_with(match)) // 如果路径部分以通配符匹配
            {
                StringPiece match_path(route.data() + cursor); // 提取匹配路径
                route_match_path = mid.as_string() + match_path.as_string(); // 构造完整匹配路径
                return iterator{kv.second, route, kv.second->verb_handler_}; // 返回匹配的迭代器
            }
        }

        if (param.size() > 2 && param[0] == '{' && param[param.size() - 1] == '}') // 如果是路径参数
        {
            int i = 1; // 去掉开头的 '{'
            int j = param.size() - 2; // 去掉结尾的 '}'
            while (param[i] == ' ') i++; // 去掉前导空格
            while (param[j] == ' ') j--; // 去掉尾部空格

            param.shrink(i, param.size() - 1 - j); // 去掉多余空格
            route_params[param.as_string()] = mid.as_string(); // 将路径参数添加到路由参数映射中
            return kv.second->find(route, cursor, route_params, route_match_path); // 递归查找
        }
    }
    return end(); // 如果没有找到匹配的路由，返回结束迭代器
}

void RouteTableNode::print_node_arch()
{
    // 使用队列进行层序遍历打印节点
    std::queue<std::pair<StringPiece, RouteTableNode *>> node_queue;
    StringPiece root("/");
    node_queue.push({root, this}); // 将根节点加入队列
    int level = 0; // 当前层级

    while (!node_queue.empty())
    {
        size_t queue_size = node_queue.size(); 
        spdlog::info("[YUKINO] level {0}: (size : {1})", level, queue_size);

        for (size_t i = 0; i < queue_size; i++)
        {
            // 获取队列中的当前节点
            std::pair<StringPiece, RouteTableNode *> node = node_queue.front();
            node_queue.pop();

            // 打印当前节点的路径
            spdlog::info("[YUKINO] current node: {0}", node.first.as_string().c_str());

            // 获取当前节点的子节点
            const std::map<StringPiece, RouteTableNode *> &children = node.second->children_;

            if (children.empty())
                spdlog::info("[YUKINO] No Children");

            // 遍历子节点并打印
            for (const auto &pair : children)
            {
                spdlog::info("[YUKINO] child node: {0}", pair.first.as_string().c_str());
                // 将子节点加入队列
                node_queue.push({pair.first, pair.second});
            }
        }
        level++; // 层级加1
    }
}

VerbHandler &RouteTable::find_or_create(const char *route)
{
    // 使用指针防止迭代器失效
    // StringPiece 只是一个观察者，因此我们需要存储字符串
    StringPiece route_piece(route);
    auto it = string_pieces_.find(route_piece); // 查找路由字符串是否已存在
    if (it != string_pieces_.end())
    {
        // 如果已存在，直接返回对应的处理器
        return root_.find_or_create(*it, 0);
    }

    // 如果不存在，将路由字符串插入集合
    string_pieces_.insert(route_piece);
    // 调用根节点的 find_or_create 方法创建或查找处理器
    return root_.find_or_create(route_piece, 0);
}



