#ifndef YUKINO_VERBHANDLER_H_
#define YUKINO_VERBHANDLER_H_

#include <functional>
#include <set>
#include "HttpMsg.h"

namespace Yukino
{

// 定义 HTTP 动词枚举
enum class Verb
{
    ANY,    // 任意动词
    GET,    // GET 请求
    POST,   // POST 请求
    PUT,    // PUT 请求
    DELETE, // DELETE 请求
    HEAD,   // HEAD 请求
    PATCH,  // PATCH 请求
};

// 将字符串转换为 Verb 枚举
inline Verb str_to_verb(const std::string &verb)
{
    if (strcasecmp(verb.c_str(), "GET") == 0)
        return Verb::GET;
    if (strcasecmp(verb.c_str(), "PUT") == 0)
        return Verb::PUT;
    if (strcasecmp(verb.c_str(), "POST") == 0)
        return Verb::POST;
    if (strcasecmp(verb.c_str(), "DELETE") == 0)
        return Verb::DELETE;
    if (strcasecmp(verb.c_str(), "HEAD") == 0)
        return Verb::HEAD;
    if (strcasecmp(verb.c_str(), "PATCH") == 0)
        return Verb::PATCH;
    return Verb::ANY; // 如果不匹配任何已知动词，返回 ANY
}

// 将 Verb 枚举转换为字符串
inline const char *verb_to_str(const Verb &verb)
{
    switch (verb)
    {
        case Verb::ANY:
            return "ANY";
        case Verb::GET:
            return "GET";
        case Verb::POST:
            return "POST";
        case Verb::PUT:
            return "PUT";
        case Verb::DELETE:
            return "DELETE";
        case Verb::HEAD:
            return "HEAD";
        case Verb::PATCH:
            return "PATCH";
        default:
            return "[UNKNOWN]";
    }
}

// 定义处理器函数类型
using WrapHandler = std::function<WFGoTask *(HttpReq *, HttpResp *, SeriesWork *)>;

// 定义动词处理器结构体
struct VerbHandler
{
    std::map<Verb, WrapHandler> verb_handler_map; // 动词到处理器的映射
    StringPiece path;                            // 路由路径（服务端注册的路径）
    int compute_queue_id;                        // 计算队列 ID
};

}  // namespace Yukino

#endif // YUKINO_VERBHANDLER_H_