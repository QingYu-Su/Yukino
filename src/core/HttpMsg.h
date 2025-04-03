#ifndef YUKINO_HTTPMSG_H_
#define YUKINO_HTTPMSG_H_

#include "workflow/HttpMessage.h"
#include "workflow/WFTaskFactory.h"

#include <fcntl.h>
#include <unordered_map>
#include <memory>

#include "StringPiece.h"
#include "HttpDef.h"
#include "HttpContent.h"
#include "Compress.h"
#include "StrUtil.h"
#include "HttpCookie.h"
#include "Noncopyable.h"
#include "HttpFile.h"
#include "Json.h"

namespace protocol
{
    class MySQLResultCursor; // 前向声明 MySQLResultCursor 类
}  // namespace protocol

namespace Yukino
{
    struct ReqData; // 前向声明 ReqData 结构体
    class MySQL; // 前向声明 MySQL 类

    /**
     * @brief HttpReq 类，表示 HTTP 请求对象
     * 
     * 该类继承自 protocol::HttpRequest 并且是不可拷贝的（继承自 Noncopyable）。
     * 它封装了 HTTP 请求的相关信息，如请求体、表单数据、JSON 数据、请求头、查询参数等。
     * 同时提供了对请求路径、路由参数、Cookie 等的访问接口。
     */
    class HttpReq : public protocol::HttpRequest, public Noncopyable
    {
    public:
        /**
         * @brief 获取请求体内容
         * 
         * @return std::string& 请求体内容的引用
         */
        std::string &body() const;

        /**
         * @brief 获取 POST 请求的表单数据，body类型为application/x-www-form-urlencoded
         * 
         * @return std::map<std::string, std::string>& 表单数据的键值对
         */
        std::map<std::string, std::string> &form_kv() const;

        /**
         * @brief 获取 POST 请求的表单对象，body类型为multipart/form-data
         * Form是一个map，其中每个元素都是一个表单对象，字段名为key，文件名和文件内容为value（本身是一对pair）
         * 
         * @return Form& 表单对象的引用
         */
        Form &form() const;

        /**
         * @brief 获取请求体中的 JSON 数据
         * 
         * @return Yukino::Json& JSON 数据的引用
         */
        Yukino::Json &json() const;

        /**
         * @brief 获取请求的内容类型
         * 
         * @return http_content_type 请求的内容类型
         */
        http_content_type content_type() const
        { return content_type_; }

        /**
         * @brief 获取请求头中的某个值
         * 
         * @param key 请求头的键
         * @return const std::string& 请求头的值
         */
        const std::string &header(const std::string &key) const;

        /**
         * @brief 检查请求头中是否存在某个键
         * 
         * @param key 请求头的键
         * @return bool 是否存在该键
         */
        bool has_header(const std::string &key) const;

        /**
         * @brief 获取路由参数中的某个值
         * 
         * @param key 路由参数的键
         * @return const std::string& 请求参数的值
         */
        const std::string &param(const std::string &key) const;

        /**
         * @brief 获取路由参数中的某个值，并将其转换为指定类型
         * 
         * @tparam T 要转换的目标类型
         * @param key 请求参数的键
         * @return T 转换后的值
         */
        template<typename T>
        T param(const std::string &key) const;

        /**
         * @brief 检查请求参数中是否存在某个键
         * 
         * @param key 请求参数的键
         * @return bool 是否存在该键
         */
        bool has_param(const std::string &key) const;

        /**
         * @brief 获取查询字符串中的某个值
         * 
         * @param key 查询字符串的键
         * @return const std::string& 查询字符串的值
         */
        const std::string &query(const std::string &key) const;

        /**
         * @brief 获取查询字符串中的某个值，如果不存在则返回默认值
         * 
         * @param key 查询字符串的键
         * @param default_val 默认值
         * @return const std::string& 查询字符串的值或默认值
         */
        const std::string &default_query(const std::string &key,
                                         const std::string &default_val) const;

        /**
         * @brief 获取查询字符串的所有键值对
         * 
         * @return const std::map<std::string, std::string>& 查询字符串的键值对
         */
        const std::map<std::string, std::string> &query_list() const
        { return query_params_; }

        /**
         * @brief 检查查询字符串中是否存在某个键
         * 
         * @param key 查询字符串的键
         * @return bool 是否存在该键
         */
        bool has_query(const std::string &key) const;

        /**
         * @brief 获取匹配的路由路径
         * 
         * @return const std::string& 匹配的路由路径
         */
        const std::string &match_path() const
        { return route_match_path_; }

        /**
         * @brief 获取服务端注册完整的路由路径
         * 
         * @return const std::string& 完整的路由路径
         */
        const std::string &full_path() const
        { return route_full_path_; }

        /**
         * @brief 获取客户端当前请求的路径
         * 
         * @return std::string 当前请求的路径
         */
        std::string current_path() const
        { return parsed_uri_.path; }

        /**
         * @brief 获取请求中的 Cookie
         * 
         * @param key Cookie 的键
         * @return const std::string& Cookie 的值
         */
        const std::string &cookie(const std::string &key) const;

        /**
         * @brief 获取请求中的所有 Cookie
         * 
         * @return const std::map<std::string, std::string>& 所有 Cookie 的键值对
         */
        const std::map<std::string, std::string> &cookies() const;

    public:
        /**
         * @brief 如果内容类型是 multipart/form-data，则填充multi_part_的边界字符串
         */
        void fill_content_type();

        /**
         * @brief 填充请求头映射
         */
        void fill_header_map();

        /**
         * @brief 设置路由参数
         * 
         * @param params 路由参数的键值对
         */
        void set_route_params(std::map<std::string, std::string> &&params)
        { route_params_ = std::move(params); }

        /**
         * @brief 设置匹配的路由路径
         * 
         * @param match_path 匹配的路由路径
         */
        void set_route_match_path(const std::string &match_path)
        { route_match_path_ = match_path; }

        /**
         * @brief 设置完整的路由路径
         * 
         * @param route_full_path 完整的路由路径
         */
        void set_full_path(const std::string &route_full_path)
        { route_full_path_ = route_full_path; }

        /**
         * @brief 设置完整的路由路径（右值引用优化）
         * 
         * @param route_full_path 完整的路由路径
         */
        void set_full_path(std::string &&route_full_path)
        { route_full_path_ = std::move(route_full_path); }

        /**
         * @brief 设置查询字符串的键值对
         * 
         * @param query_params 查询字符串的键值对
         */
        void set_query_params(std::map<std::string, std::string> &&query_params)
        { query_params_ = std::move(query_params); }

        /**
         * @brief 设置解析后的 URI
         * 
         * @param parsed_uri 解析后的 URI
         */
        void set_parsed_uri(ParsedURI &&parsed_uri)
        { parsed_uri_ = std::move(parsed_uri); }

    public:
        /**
         * @brief 构造函数
         */
        HttpReq();

        /**
         * @brief 构造函数（基于 HttpRequest 对象）
         * 
         * @param base_req HttpRequest 对象
         */
        HttpReq(HttpRequest &&base_req)
            : HttpRequest(std::move(base_req))
        {}

        /**
         * @brief 析构函数
         */
        ~HttpReq();

        /**
         * @brief 移动构造函数
         * 
         * @param other 要移动的对象
         */
        HttpReq(HttpReq&& other);

        /**
         * @brief 移动赋值运算符
         * 
         * @param other 要移动的对象
         * @return HttpReq& 当前对象的引用
         */
        HttpReq &operator=(HttpReq&& other);

    private:
        // 定义一个类型别名 HeaderMap，用于表示 HTTP请求头部的映射结构，并且键值通过MapStringCaseLess使其不区分大小写
        using HeaderMap = std::map<std::string, std::vector<std::string>, MapStringCaseLess>;

        http_content_type content_type_; // 请求的内容类型
        ReqData *req_data_; // 请求数据指针

        std::string route_match_path_; // 匹配的路由路径
        std::string route_full_path_; // 完整的路由路径

        std::map<std::string, std::string> route_params_; // 路由参数
        std::map<std::string, std::string> query_params_; // 查询字符串的键值对
        mutable std::map<std::string, std::string> cookies_; // Cookie 的键值对

        MultiPartForm multi_part_; // 多部分表单
        HeaderMap headers_; // 请求头映射

        ParsedURI parsed_uri_; // 解析后的 URI 信息
};

// 特化模板函数，将字符串参数转换为 int 类型
template<>
inline int HttpReq::param<int>(const std::string &key) const
{
    // 检查 route_params_ 中是否存在指定的键
    if (route_params_.count(key))
        // 如果存在，将对应的字符串值转换为 int 类型
        return std::stoi(route_params_.at(key));
    else
        // 如果不存在，返回默认值 0
        return 0;
}

// 特化模板函数，将字符串参数转换为 size_t 类型
template<>
inline size_t HttpReq::param<size_t>(const std::string &key) const
{
    // 检查 route_params_ 中是否存在指定的键
    if (route_params_.count(key))
        // 如果存在，将对应的字符串值转换为 unsigned long 类型，然后强制转换为 size_t
        return static_cast<size_t>(std::stoul(route_params_.at(key)));
    else
        // 如果不存在，返回默认值 0
        return 0;
}

// 特化模板函数，将字符串参数转换为 double 类型
template<>
inline double HttpReq::param<double>(const std::string &key) const
{
    // 检查 route_params_ 中是否存在指定的键
    if (route_params_.count(key))
        // 如果存在，将对应的字符串值转换为 double 类型
        return std::stod(route_params_.at(key));
    else
        // 如果不存在，返回默认值 0.0
        return 0.0;
}

/**
 * @brief HttpResp 类，表示 HTTP 响应对象
 * 
 * 该类继承自 protocol::HttpResponse 并且是不可拷贝的（继承自 Noncopyable）。
 * 它封装了 HTTP 响应的相关信息，如响应头、Cookie、压缩方式等。
 * 同时提供了发送字符串、文件、JSON 响应，代理请求，MySQL 和 Redis 请求，重定向，错误响应，定时器，推送等功能。
 */
class HttpResp : public protocol::HttpResponse, public Noncopyable
{
public:
    using MySQLJsonFunc = std::function<void(Yukino::Json *json)>; // MySQL JSON 回调函数类型

    using MySQLFunc = std::function<void(protocol::MySQLResultCursor *cursor)>; // MySQL 回调函数类型

    using RedisJsonFunc = std::function<void(Yukino::Json *json)>; // Redis JSON 回调函数类型

    using RedisFunc = std::function<void(WFRedisTask *redis_task)>; // Redis 回调函数类型

    using TimerFunc = std::function<void()>; // 定时器回调函数类型

    using PushFunc = std::function<void(std::string &ctx)>; // 推送回调函数类型

    using PushErrorFunc = std::function<void()>; // 推送错误回调函数类型

public:
    // 发送字符串响应
    void String(const std::string &str);

    void String(std::string &&str);

    void String(MultiPartEncoder &&encoder);

    void String(const MultiPartEncoder &encoder);

    // 发送文件响应
    void File(const std::string &path);

    void File(const std::string &path, size_t start);

    void File(const std::string &path, size_t start, size_t end);

    // 保存文件
    void Save(const std::string &file_dst, const std::string &content);

    void Save(const std::string &file_dst, const std::string &content, const std::string &notify_msg);

    void Save(const std::string &file_dst, const std::string &content,
              const HttpFile::FileIOArgsFunc &func);

    void Save(const std::string &file_dst, std::string &&content);

    void Save(const std::string &file_dst, std::string &&content, const std::string &notify_msg);

    void Save(const std::string &file_dst, std::string &&content,
              const HttpFile::FileIOArgsFunc &func);

    // 发送 JSON 响应
    void Json(const Yukino::Json &json);

    void Json(const std::string &str);

    // 设置响应状态码
    void set_status(int status_code);

    // 设置压缩方式
    void set_compress(const Compress &compress);

    // 添加 Cookie
    void add_cookie(HttpCookie &&cookie)
    { cookies_.emplace_back(std::move(cookie)); }

    void add_cookie(const HttpCookie &cookie)
    { cookies_.push_back(cookie); }

    const std::vector<HttpCookie> &cookies() const
    { return cookies_; }

    // 获取响应状态
    int get_state() const;

    // 获取错误码
    int get_error() const;

    // 代理请求
    void Http(const std::string &url, int redirect_max, size_t size_limit);

    void Http(const std::string &url, int redirect_max)
    { this->Http(url, redirect_max, 200 * 1024 * 1024); }

    void Http(const std::string &url)
    { this->Http(url, 0, 200 * 1024 * 1024); }

    // MySQL 请求
    void MySQL(const std::string &url, const std::string &sql);

    void MySQL(const std::string &url, const std::string &sql, const MySQLJsonFunc &func);

    void MySQL(const std::string &url, const std::string &sql, const MySQLFunc &func);

    // Redis 请求
    void Redis(const std::string &url, const std::string &command,
            const std::vector<std::string>& params);

    void Redis(const std::string &url, const std::string &command,
            const std::vector<std::string>& params, const RedisJsonFunc &func);

    void Redis(const std::string &url, const std::string &command,
            const std::vector<std::string>& params, const RedisFunc &func);

    // 重定向
    void Redirect(const std::string& location, int status_code);

    // 在指定的计算队列中执行计算任务
    template<class FUNC, class... ARGS>
    void Compute(int compute_queue_id, FUNC&& func, ARGS&&... args)
    {
        // 创建一个计算任务
        WFGoTask *go_task = WFTaskFactory::create_go_task(
                "Yukino" + std::to_string(compute_queue_id), // 任务名称
                std::forward<FUNC>(func), // 通过完美转发传递计算函数
                std::forward<ARGS>(args)...); // 通过完美转发传递函数参数

        // 将计算任务添加到任务列表中
        this->add_task(go_task);
    }

    // 错误响应
    void Error(int error_code);

    void Error(int error_code, const std::string &errmsg);

    // 定时器（睡眠）
    void Timer(unsigned int microseconds, const TimerFunc &cb);

    void Timer(time_t seconds, long nanoseconds, const TimerFunc& cb);

    // 推送
    void Push(const std::string &cond_name, const PushFunc &cb);

    void Push(const std::string &cond_name, const PushFunc &cb, const PushErrorFunc &err_cb);

    // 添加子任务
    void add_task(SubTask *task);

    // 添加响应头
    void add_header(const std::string &key, const std::string &val)
    {
        headers[key] = val;
    }
private:
    // 压缩数据
    int compress(const std::string * const data, std::string *compress_data);

    // 构造推送头部
    std::string construct_push_header();

    // 发送字符串响应（多部分编码器）
    void String(MultiPartEncoder *encoder);

    public:
    // 默认构造函数
    HttpResp() = default;
    // 说明：默认构造函数，用于创建一个空的 HttpResp 对象。

    // 构造函数，基于 protocol::HttpResponse 对象进行初始化
    HttpResp(HttpResponse && base_resp)
        : HttpResponse(std::move(base_resp))
    {}
    // 说明：通过右值引用接收一个 HttpResponse 对象，并将其移动到当前 HttpResp 对象中。
    //       这允许 HttpResp 继承并复用 HttpResponse 的资源，避免不必要的拷贝。

    // 默认析构函数
    ~HttpResp() = default;
    // 说明：默认析构函数，用于释放 HttpResp 对象占用的资源。

    // 移动构造函数
    HttpResp(HttpResp&& other);
    // 说明：移动构造函数，用于将另一个 HttpResp 对象的资源移动到当前对象中。
    //       这允许在对象移动时高效地复用资源，而不是进行深拷贝。

    // 移动赋值运算符
    HttpResp &operator=(HttpResp&& other);
    // 说明：移动赋值运算符，用于将另一个 HttpResp 对象的资源移动到当前对象中。
    //       这允许在对象赋值时高效地复用资源，而不是进行深拷贝。

public:
    std::map<std::string, std::string, MapStringCaseLess> headers; // 响应头映射
    void *user_data; // 用户数据指针

private:
    std::vector<HttpCookie> cookies_; // Cookie 列表
};

// 定义一个类型别名 HttpTask，表示基于 HttpReq 和 HttpResp 的网络任务。
using HttpTask = WFNetworkTask<HttpReq, HttpResp>;

// 定义一个内联函数 sse_signal，通过条件名称唤醒所有相同名称的条件任务
inline void sse_signal(const std::string& cond_name)
{
    WFTaskFactory::signal_by_name(cond_name, NULL);
}

} // namespace Yukino


#endif // YUKINO_HTTPMSG_H_
