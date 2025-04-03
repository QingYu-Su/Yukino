#include "workflow/HttpUtil.h"
#include "workflow/MySQLResult.h"
#include "workflow/WFMySQLConnection.h"
#include "workflow/Workflow.h"
#include "workflow/WFTaskFactory.h"

#include <unistd.h>
#include <algorithm>

#include "HttpMsg.h"
#include "UriUtil.h"
#include "PathUtil.h"
#include "MysqlUtil.h"
#include "ErrorCode.h"
#include "FileUtil.h"
#include "HttpServerTask.h"
#include "CodeUtil.h"
#include "spdlog/spdlog.h" 

using namespace protocol;

namespace Yukino
{

// 请求数据结构体
struct ReqData
{
    std::string body; // 请求体内容
    std::map<std::string, std::string> form_kv; // 表单数据的键值对
    Form form; // 表单对象
    Json json; // JSON 数据
};

// 代理上下文结构体
struct ProxyCtx
{
    std::string url; // 代理目标 URL
    HttpServerTask *server_task; // 指向 HttpServerTask 的指针
    bool is_keep_alive; // 是否保持连接
};

// HTTP 代理回调函数
void proxy_http_callback(WFHttpTask *http_task)
{
    // 获取任务的状态和错误码
    int state = http_task->get_state();
    int error = http_task->get_error();

    // 获取代理上下文
    auto *proxy_ctx = static_cast<ProxyCtx *>(http_task->user_data);

    // 获取 HttpServerTask 和 HttpResp 对象
    HttpServerTask *server_task = proxy_ctx->server_task;
    HttpResponse *http_resp = http_task->get_resp();
    HttpResp *server_resp = server_task->get_resp();

    // 如果服务器关闭了连接，将状态设置为成功
    if (state == WFT_STATE_SYS_ERROR && error == ECONNRESET)
        state = WFT_STATE_SUCCESS;

    // 如果任务成功完成
    if (state == WFT_STATE_SUCCESS)
    {
        // 添加回调函数，用于在任务完成后处理响应
        // 此时的问题是目标服务器能正常回复，但本地无法将其转发给客户端
        server_task->add_callback([proxy_ctx](HttpTask *server_task)
        {
            HttpResp *server_resp = server_task->get_resp();
            size_t size = server_resp->get_output_body_size();
            if (server_task->get_state() != WFT_STATE_SUCCESS)
            {
                // 如果任务失败，设置错误信息
                std::string errmsg;
                errmsg.reserve(64);
                errmsg.append(proxy_ctx->url);
                errmsg.append(" : Reply failed: ");
                errmsg.append(strerror(server_task->get_error()));
                errmsg.append(", BodyLength: ");
                errmsg.append(std::to_string(size));
                server_resp->Error(StatusProxyError, errmsg);
            }
        });

        // 获取远程服务器的响应体
        const void *body;
        size_t len;
        if (http_resp->get_parsed_body(&body, &len))
            http_resp->append_output_body_nocopy(body, len);

        // 将远程服务器的响应复制到服务器响应中
        HttpResp resp(std::move(*http_resp));
        *server_resp = std::move(resp);

        // 如果不是保持连接，设置 Connection 头为 close
        if (!proxy_ctx->is_keep_alive)
            server_resp->set_header_pair("Connection", "close");
    }
    else
    {
        // 如果任务失败，设置错误信息
        const char *err_string;
        int error = http_task->get_error();

        if (state == WFT_STATE_SYS_ERROR)
            err_string = strerror(error);
        else if (state == WFT_STATE_DNS_ERROR)
            err_string = gai_strerror(error);
        else if (state == WFT_STATE_SSL_ERROR)
            err_string = "SSL error";
        else /* if (state == WFT_STATE_TASK_ERROR) */
            err_string = "URL error (Cannot be a HTTPS proxy)";

        std::string errmsg;
        errmsg.reserve(64);
        errmsg.append(proxy_ctx->url);
        errmsg.append(" : Fetch failed. state = ");
        errmsg.append(std::to_string(state));
        errmsg.append(", error = ");
        errmsg.append(std::to_string(http_task->get_error()));
        errmsg.append(" ");
        errmsg.append(err_string);
        server_resp->Error(StatusProxyError, errmsg);
    }

    // 添加回调函数，用于在任务完成后释放代理上下文
    server_task->add_callback([proxy_ctx](HttpTask *server_task)
    {
        delete proxy_ctx;
    });

    // 将请求对象移回服务器任务
    auto *server_req = static_cast<HttpRequest *>(server_task->get_req());
    *server_req = std::move(*http_task->get_req());
}

Yukino::Json mysql_concat_json_res(WFMySQLTask *mysql_task)
{
    Yukino::Json json; // 创建一个 JSON 对象，用于存储最终结果
    MySQLResponse *mysql_resp = mysql_task->get_resp(); // 获取 MySQL 响应对象
    MySQLResultCursor cursor(mysql_resp); // 创建一个 MySQL 结果游标
    const MySQLField *const *fields; // 用于存储字段信息
    std::vector<MySQLCell> arr; // 用于存储行数据

    // 如果任务失败，返回错误信息
    if (mysql_task->get_state() != WFT_STATE_SUCCESS)
    {
        json["error"] = WFGlobal::get_error_string(mysql_task->get_state(),
                                                   mysql_task->get_error());
        return json;
    }

    // 遍历所有结果集
    do {
        Yukino::Json result_set; // 创建一个 JSON 对象，用于存储当前结果集
        if (cursor.get_cursor_status() != MYSQL_STATUS_GET_RESULT &&
            cursor.get_cursor_status() != MYSQL_STATUS_OK)
        {
            break; // 如果游标状态不是获取结果或成功，退出循环
        }

        // 如果游标状态是获取结果
        if (cursor.get_cursor_status() == MYSQL_STATUS_GET_RESULT)
        {
            result_set["field_count"] = cursor.get_field_count(); // 字段数量
            result_set["rows_count"] = cursor.get_rows_count(); // 行数量
            fields = cursor.fetch_fields(); // 获取字段信息
            std::vector<std::string> fields_name; // 字段名称
            std::vector<std::string> fields_type; // 字段类型
            for (int i = 0; i < cursor.get_field_count(); i++)
            {
                if (i == 0)
                {
                    std::string database = fields[i]->get_db(); // 数据库名称
                    if (!database.empty())
                        result_set["database"] = std::move(database);
                    result_set["table"] = fields[i]->get_table(); // 表名称
                }

                fields_name.push_back(fields[i]->get_name()); // 字段名称
                fields_type.push_back(datatype2str(fields[i]->get_data_type())); // 字段类型
            }
            result_set["fields_name"] = fields_name; // 字段名称数组
            result_set["fields_type"] = fields_type; // 字段类型数组

            // 遍历所有行
            while (cursor.fetch_row(arr))
            {
                Yukino::Json row; // 创建一个 JSON 对象，用于存储当前行
                for (size_t i = 0; i < arr.size(); i++)
                {
                    if (arr[i].is_string())
                    {
                        row.push_back(arr[i].as_string()); // 字符串类型
                    }
                    else if (arr[i].is_time() || arr[i].is_datetime())
                    {
                        row.push_back(MySQLUtil::to_string(arr[i])); // 时间或日期时间类型
                    }
                    else if (arr[i].is_null())
                    {
                        row.push_back("NULL"); // NULL 类型
                    }
                    else if (arr[i].is_double())
                    {
                        row.push_back(arr[i].as_double()); // 双精度浮点类型
                    }
                    else if (arr[i].is_float())
                    {
                        row.push_back(arr[i].as_float()); // 浮点类型
                    }
                    else if (arr[i].is_int())
                    {
                        row.push_back(arr[i].as_int()); // 整数类型
                    }
                    else if (arr[i].is_ulonglong())
                    {
                        row.push_back(arr[i].as_ulonglong()); // 无符号长整数类型
                    }
                }
                result_set["rows"].push_back(row); // 将当前行添加到结果集中
            }
        }
        else if (cursor.get_cursor_status() == MYSQL_STATUS_OK)
        {
            result_set["status"] = "OK"; // 状态
            result_set["affected_rows"] = cursor.get_affected_rows(); // 影响的行数
            result_set["warnings"] = cursor.get_warnings(); // 警告数量
            result_set["insert_id"] = cursor.get_insert_id(); // 插入的 ID
            result_set["info"] = cursor.get_info(); // 附加信息
        }
        json["result_set"].push_back(result_set); // 将当前结果集添加到最终结果中
    } while (cursor.next_result_set()); // 转到下一个结果集

    // 如果 MySQL 响应是错误包
    if (mysql_resp->get_packet_type() == MYSQL_PACKET_ERROR)
    {
        json["errcode"] = mysql_task->get_resp()->get_error_code(); // 错误代码
        json["errmsg"] = mysql_task->get_resp()->get_error_msg(); // 错误消息
    }
    else if (mysql_resp->get_packet_type() == MYSQL_PACKET_OK)
    {
        json["status"] = "OK"; // 状态
        json["affected_rows"] = mysql_task->get_resp()->get_affected_rows(); // 影响的行数
        json["warnings"] = mysql_task->get_resp()->get_warnings(); // 警告数量
        json["insert_id"] = mysql_task->get_resp()->get_last_insert_id(); // 插入的 ID
        json["info"] = mysql_task->get_resp()->get_info(); // 附加信息
    }
    return json; // 返回最终结果
}

Yukino::Json redis_json_res(WFRedisTask *redis_task)
{
    // 获取 Redis 请求和响应对象
    RedisRequest *redis_req = redis_task->get_req();
    RedisResponse *redis_resp = redis_task->get_resp();
    int state = redis_task->get_state(); // 获取任务状态
    int error = redis_task->get_error(); // 获取错误码
    RedisValue val; // 用于存储 Redis 响应值
    Yukino::Json js; // 创建一个 JSON 对象，用于存储最终结果

    // 根据任务状态处理不同的情况
    switch (state)
    {
    case WFT_STATE_SYS_ERROR:
        // 系统错误
        js["errmsg"] = "system error: " + std::string(strerror(error));
        break;
    case WFT_STATE_DNS_ERROR:
        // DNS 错误
        js["errmsg"] = "DNS error: " + std::string(gai_strerror(error));
        break;
    case WFT_STATE_SSL_ERROR:
        // SSL 错误
        js["errmsg"] = "SSL error: " + std::to_string(error);
        break;
    case WFT_STATE_TASK_ERROR:
        // 任务错误
        js["errmsg"] = "Task error: " + std::to_string(error);
        break;
    case WFT_STATE_SUCCESS:
        // 任务成功
        redis_resp->get_result(val); // 获取 Redis 响应结果
        if (val.is_error())
        {
            // 如果 Redis 响应是错误
            js["errmsg"] = "Error reply. Need a password?\n";
            state = WFT_STATE_TASK_ERROR; // 将状态设置为任务错误
        }
        break;
    }

    // 获取 Redis 请求的命令和参数
    std::string cmd;
    std::vector<std::string> params;
    redis_req->get_command(cmd);
    redis_req->get_params(params);

    // 根据命令类型处理不同的情况
    if (state == WFT_STATE_SUCCESS && cmd == "SET")
    {
        // 如果命令是 SET
        js["status"] = "success"; // 设置状态为成功
        js["cmd"] = "SET"; // 设置命令类型
        js[params[0]] = params[1]; // 将键值对添加到 JSON 中
    }
    if (state == WFT_STATE_SUCCESS && cmd == "GET")
    {
        // 如果命令是 GET
        js["cmd"] = "GET"; // 设置命令类型
        if (val.is_string())
        {
            // 如果响应值是字符串
            js[params[0]] = val.string_value(); // 将值添加到 JSON 中
            js["status"] = "success"; // 设置状态为成功
        }
        else
        {
            // 如果响应值不是字符串
            js["errmsg"] = "value is not a string value";
        }
    }
    return js; // 返回最终结果
}

// MySQL 任务完成后的回调函数
void mysql_callback(WFMySQLTask *mysql_task)
{
    // 将 MySQL 查询结果转换为 JSON 格式
    Yukino::Json json = mysql_concat_json_res(mysql_task);

    // 从 mysql_task 的 user_data 中获取 HttpResp 对象
    auto *server_resp = static_cast<HttpResp *>(mysql_task->user_data);

    // 将 JSON 数据作为字符串发送到 HTTP 响应中
    server_resp->String(json.dump());
}

// HttpReq 类的构造函数
HttpReq::HttpReq() : req_data_(new ReqData)
{
    // 构造函数中初始化 req_data_，分配一个新的 ReqData 对象
}

// HttpReq 类的析构函数
HttpReq::~HttpReq()
{
    // 析构函数中释放 req_data_ 所指向的 ReqData 对象
    delete req_data_;
}

// 获取 HTTP 请求体内容
std::string &HttpReq::body() const
{
    // 如果请求体内容为空，则进行解码和解压处理
    if (req_data_->body.empty())
    {
        // 解码分块传输编码的请求体内容
        std::string content = protocol::HttpUtil::decode_chunked_body(this);

        // 获取请求头中的 Content-Encoding 字段
        const std::string &header = this->header("Content-Encoding");

        // 初始化状态码为成功
        int status = StatusOK;

        // 检查是否为 gzip 压缩的请求体
        if (header.find("gzip") != std::string::npos)
        {
            // 尝试解压 gzip 压缩的请求体
            status = Compressor::ungzip(&content, &req_data_->body);
        }
        else
        {
            // 如果不是 gzip 压缩，设置状态码为未压缩
            status = StatusNoUncomrpess;
        }

        // 如果解压失败，则直接使用原始内容
        if(status != StatusOK)
        {
            req_data_->body = std::move(content);
        }
    }
    // 返回请求体内容
    return req_data_->body;
}

// 获取 HTTP 请求的表单键值对
std::map<std::string, std::string> &HttpReq::form_kv() const
{
    // 如果请求的内容类型为 application/x-www-form-urlencoded 且表单键值对为空，则解析表单数据
    if (content_type_ == APPLICATION_URLENCODED && req_data_->form_kv.empty())
    {
        // 获取请求体内容
        StringPiece body_piece(this->body());

        // 解析表单数据为键值对
        req_data_->form_kv = Urlencode::parse_post_kv(body_piece);
    }
    // 返回表单键值对
    return req_data_->form_kv;
}

// 获取 HTTP 请求的表单对象
Form &HttpReq::form() const
{
    // 如果请求的内容类型为 multipart/form-data 且表单对象为空，则解析表单数据
    if (content_type_ == MULTIPART_FORM_DATA && req_data_->form.empty())
    {
        // 获取请求体内容
        StringPiece body_piece(this->body());

        // 解析表单数据为表单对象
        req_data_->form = multi_part_.parse_multipart(body_piece);
    }
    // 返回表单对象
    return req_data_->form;
}

// 获取 HTTP 请求中的 JSON 数据
Yukino::Json &HttpReq::json() const
{
    // 如果请求的内容类型是 JSON 且 JSON 数据为空
    if (content_type_ == APPLICATION_JSON && req_data_->json.empty())
    {
        // 获取请求体内容
        const std::string &body_content = this->body();
        // 尝试解析 JSON 数据
        Json tmp = Json::parse(body_content);
        // 如果解析成功
        if (tmp.is_valid())
        {
            // 将解析后的 JSON 数据移动到 req_data_->json 中
            req_data_->json = std::move(tmp);
        }
    }
    // 返回 JSON 数据
    return req_data_->json;
}

// 获取路由参数中的值
const std::string &HttpReq::param(const std::string &key) const
{
    // 如果路由参数中存在该键
    if (route_params_.count(key))
        return route_params_.at(key); // 返回对应的值
    else
        return string_not_found; // 否则返回一个表示未找到的字符串
}

// 检查路由参数中是否存在某个键
bool HttpReq::has_param(const std::string &key) const
{
    return route_params_.count(key) > 0; // 如果存在该键，返回 true
}

// 获取查询字符串中的值
const std::string &HttpReq::query(const std::string &key) const
{
    // 如果查询字符串中存在该键
    if (query_params_.count(key))
        return query_params_.at(key); // 返回对应的值
    else
        return string_not_found; // 否则返回一个表示未找到的字符串
}

// 获取查询字符串中的值，如果不存在则返回默认值
const std::string &HttpReq::default_query(const std::string &key, const std::string &default_val) const
{
    // 如果查询字符串中存在该键
    if (query_params_.count(key))
        return query_params_.at(key); // 返回对应的值
    else
        return default_val; // 否则返回默认值
}

// 检查查询字符串中是否存在某个键
bool HttpReq::has_query(const std::string &key) const
{
    return query_params_.find(key) != query_params_.end(); // 如果存在该键，返回 true
}

// 如果内容类型是 multipart/form-data，则填充multi_part_的边界字符串
void HttpReq::fill_content_type()
{
    // 获取请求头中的 Content-Type 字段
    const std::string &content_type_str = header("Content-Type");
    // 将内容类型字符串转换为枚举值
    content_type_ = ContentType::to_enum(content_type_str);

    // 如果内容类型是 multipart/form-data
    if (content_type_ == MULTIPART_FORM_DATA)
    {
        // 尝试找到 boundary 参数
        const char *boundary = strstr(content_type_str.c_str(), "boundary=");
        if (boundary == nullptr)
        {
            return; // 如果没有找到 boundary 参数，直接返回
        }
        boundary += strlen("boundary="); // 跳过 "boundary=" 字符串
        StringPiece boundary_piece(boundary); // 创建一个 StringPiece 对象

        // 去掉 boundary 参数值的引号
        StringPiece boundary_str = StrUtil::trim_pairs(boundary_piece, R"(""'')");
        // 设置多部分表单的边界
        multi_part_.set_boundary(boundary_str.as_string());
    }
}

// 获取 HTTP 请求头中的某个值
const std::string &HttpReq::header(const std::string &key) const
{
    // 在 headers_ 中查找指定的键
    const auto it = headers_.find(key);

    // 如果未找到或对应的值为空，返回一个表示未找到的字符串
    if (it == headers_.end() || it->second.empty())
        return string_not_found;

    // 返回找到的第一个值
    return it->second[0];
}

// 检查 HTTP 请求头中是否存在某个键
bool HttpReq::has_header(const std::string &key) const
{
    // 如果 headers_ 中存在该键，返回 true
    return headers_.count(key) > 0;
}

// 填充 HTTP 请求头映射
void HttpReq::fill_header_map()
{
    // 初始化 HTTP 请求头游标
    http_header_cursor_t cursor;
    struct protocol::HttpMessageHeader header;

    http_header_cursor_init(&cursor, this->get_parser());
    // 遍历所有请求头
    while (http_header_cursor_next(&header.name, &header.name_len,
                                   &header.value, &header.value_len,
                                   &cursor) == 0)
    {
        // 构造请求头的键
        std::string key(static_cast<const char *>(header.name), header.name_len);

        // 将请求头的值添加到映射中
        headers_[key].emplace_back(static_cast<const char *>(header.value), header.value_len);
    }

    // 释放 HTTP 请求头游标
    http_header_cursor_deinit(&cursor);
}

// 获取 HTTP 请求中的所有 Cookie
const std::map<std::string, std::string> &HttpReq::cookies() const
{
    // 如果 Cookie 映射为空且存在 "Cookie" 请求头
    if (cookies_.empty() && this->has_header("Cookie"))
    {
        // 获取 "Cookie" 请求头的值
        const std::string &cookie = this->header("Cookie");
        StringPiece cookie_piece(cookie);
        // 解析 Cookie
        cookies_ = HttpCookie::split(cookie_piece);
    }
    return cookies_;
}

// 获取 HTTP 请求中的某个 Cookie 值
const std::string &HttpReq::cookie(const std::string &key) const
{
    // 如果 Cookie 映射为空，先调用 cookies() 方法填充
    if (cookies_.empty())
    {
        this->cookies();
    }
    // 如果找到指定的 Cookie 键
    if (cookies_.find(key) != cookies_.end())
    {
        return cookies_[key];
    }
    // 否则返回一个表示未找到的字符串
    return string_not_found;
}

// HttpReq 的移动构造函数
HttpReq::HttpReq(HttpReq&& other)
    : HttpRequest(std::move(other)),
    content_type_(other.content_type_),
    route_match_path_(std::move(other.route_match_path_)),
    route_full_path_(std::move(other.route_full_path_)),
    route_params_(std::move(other.route_params_)),
    query_params_(std::move(other.query_params_)),
    cookies_(std::move(other.cookies_)),
    multi_part_(std::move(other.multi_part_)),
    headers_(std::move(other.headers_)),
    parsed_uri_(std::move(other.parsed_uri_))
{
    // 移动 req_data_ 指针
    req_data_ = other.req_data_;
    other.req_data_ = nullptr;
}

// HttpReq 的移动赋值运算符
HttpReq &HttpReq::operator=(HttpReq&& other)
{
    // 调用基类的移动赋值运算符
    HttpRequest::operator=(std::move(other));
    content_type_ = other.content_type_;

    // 移动 req_data_ 指针
    req_data_ = other.req_data_;
    other.req_data_ = nullptr;

    // 移动其他成员变量
    route_match_path_ = std::move(other.route_match_path_);
    route_full_path_ = std::move(other.route_full_path_);
    route_params_ = std::move(other.route_params_);
    query_params_ = std::move(other.query_params_);
    cookies_ = std::move(other.cookies_);
    multi_part_ = std::move(other.multi_part_);
    headers_ = std::move(other.headers_);
    parsed_uri_ = std::move(other.parsed_uri_);

    return *this;
}

// 向 HTTP 响应中添加字符串内容（左值引用）
void HttpResp::String(const std::string &str)
{
    // 创建一个字符串对象用于存储压缩后的数据
    auto *compress_data = new std::string;

    // 尝试压缩字符串内容
    int ret = this->compress(&str, compress_data);

    // 如果压缩失败
    if (ret != StatusOK)
    {
        // 直接将原始字符串添加到响应体中
        this->append_output_body(static_cast<const void *>(str.c_str()), str.size());
    }
    else
    {
        // 如果压缩成功，将压缩后的数据添加到响应体中
        this->append_output_body_nocopy(compress_data->c_str(), compress_data->size());
    }

    // 添加回调函数，在任务完成后释放压缩数据
    task_of(this)->add_callback([compress_data](HttpTask *) { delete compress_data; });
}

// 向 HTTP 响应中添加字符串内容（右值引用优化）
void HttpResp::String(std::string &&str)
{
    // 创建一个字符串对象用于存储数据
    auto *data = new std::string;

    // 尝试压缩字符串内容
    int ret = this->compress(&str, data);

    // 如果压缩失败
    if (ret != StatusOK)
    {
        // 将原始字符串移动到数据对象中
        *data = std::move(str);
    }

    // 将数据添加到响应体中
    this->append_output_body_nocopy(data->c_str(), data->size());

    // 添加回调函数，在任务完成后释放数据对象
    task_of(this)->add_callback([data](HttpTask *) { delete data; });
}

// 向 HTTP 响应中添加多部分表单编码数据（常量引用）
void HttpResp::String(const MultiPartEncoder &multi_part_encoder)
{
    // 创建一个 MultiPartEncoder 对象的副本
    MultiPartEncoder *encoder = new MultiPartEncoder(multi_part_encoder);

    // 调用重载版本
    this->String(encoder);
}

// 向 HTTP 响应中添加多部分表单编码数据（右值引用优化）
void HttpResp::String(MultiPartEncoder &&multi_part_encoder)
{
    // 创建一个 MultiPartEncoder 对象，移动构造
    MultiPartEncoder *encoder = new MultiPartEncoder(std::move(multi_part_encoder));

    // 调用重载版本
    this->String(encoder);
}

// 向 HTTP 响应中添加多部分表单编码数据（指针）
void HttpResp::String(MultiPartEncoder *encoder)
{
    // 获取多部分表单的边界字符串
    const std::string &boundary = encoder->boundary();

    // 设置响应头中的 Content-Type，包含边界字符串
    this->headers["Content-Type"] = "multipart/form-data; boundary=" + boundary;

    // 获取当前的 HttpServerTask 和 SeriesWork 对象
    HttpServerTask *server_task = task_of(this);
    SeriesWork *series = series_of(server_task);

    // 创建一个字符串对象用于存储表单内容
    std::string *content = new std::string;

    // 将内容对象设置为 SeriesWork 的上下文（主要是方便创建异步文件读取任务时，可以写入到content中）
    series->set_context(content);

    // 设置回调函数，在任务完成后释放资源
    series->set_callback([encoder](const SeriesWork *series)
    {
        delete encoder; // 释放 MultiPartEncoder 对象
        delete static_cast<std::string *>(series->get_context()); // 释放内容对象
    });

    // 获取表单参数列表，其参数列表如下所示
    // boundary
    // Content-Disposition: form-data; name="abc"; filename="test.txt"
    // Content-Type: type
    // value
    const MultiPartEncoder::ParamList &param_list = encoder->params();
    int param_idx = 0; // 参数索引
    for(const auto &param : param_list)
    {
        if (param_idx != 0)
        {
            content->append("\r\n"); // 在参数之间添加换行符
        }
        param_idx++;
        content->append("--"); // 添加边界标记
        content->append(boundary);
        content->append("\r\nContent-Disposition: form-data; name=\"");
        content->append(param.first); // 参数名称
        content->append("\"\r\n\r\n");
        content->append(param.second); // 参数值
    }

    // 获取表单文件列表
    const MultiPartEncoder::FileList &file_list = encoder->files();
    size_t file_cnt = file_list.size(); // 文件数量
    assert(file_cnt >= 0); // 断言文件数量非负
    if (file_cnt == 0)
    {
        // 如果没有文件，直接添加结束标记并发送内容
        content->append("\r\n--");
        content->append(boundary);
        content->append("--\r\n");
        this->append_output_body_nocopy(content->c_str(), content->size());
    }

    // 获取文件参数列表，其参数列表如下所示
    // boundary
    // Content-Disposition: form-data; name="abc"
    // value
    size_t file_idx = 0; // 文件索引
    for(const auto &file : file_list)
    {
        // 检查文件是否存在
        if(!PathUtil::is_file(file.second))
        {
            spdlog::error("[YUKINO] Not a File : {}", file.second.c_str());
            continue;
        }

        // 获取文件大小
        size_t file_size;
        int ret = FileUtil::size(file.second, &file_size);
        if (ret != StatusOK)
        {
            spdlog::error("[YUKINO] Invalid File : {}", file.second.c_str());
            continue;
        }

        // 分配内存用于存储文件内容
        void *buf = malloc(file_size);
        server_task->add_callback([buf](const HttpTask *server_task)
                                {
                                    free(buf); // 在任务完成后释放内存
                                });

        // 创建异步文件读取任务
        WFFileIOTask *pread_task = WFTaskFactory::create_pread_task(file.second,
                buf, file_size, 0,
                [&file, &boundary, param_idx, file_idx](WFFileIOTask *pread_task)
                {
                    FileIOArgs *args = pread_task->get_args();
                    long ret = pread_task->get_retval();

                    SeriesWork *series = series_of(pread_task);
                    std::string *content = static_cast<std::string *>(series->get_context());
                    if (pread_task->get_state() != WFT_STATE_SUCCESS || ret < 0)
                    {
                        spdlog::error("[YUKINO] Read {} Error", file.second.c_str());
                    } 
                    else
                    {
                        // 获取文件后缀并确定文件类型
                        std::string file_suffix = PathUtil::suffix(file.second);
                        std::string file_type = ContentType::to_str_by_suffix(file_suffix);

                        if (param_idx != 0 || file_idx != 0)
                        {
                            content->append("\r\n"); // 在文件之间添加换行符
                        }
                        content->append("--"); // 添加边界标记
                        content->append(boundary);
                        content->append("\r\nContent-Disposition: form-data; name=\"");
                        content->append(file.first); // 文件参数名称
                        content->append("\"; filename=\"");
                        content->append(PathUtil::base(file.second)); // 文件名
                        content->append("\"\r\nContent-Type: ");
                        content->append(file_type); // 文件类型
                        content->append("\r\n\r\n");
                        content->append(static_cast<char *>(args->buf), ret); // 文件内容
                    }

                    // 如果是最后一个文件，发送内容
                    if(pread_task->user_data) 
                    {
                        content->append("\r\n--");
                        content->append(boundary);
                        content->append("--\r\n");
                        HttpResp *resp = static_cast<HttpResp *>(pread_task->user_data);
                        resp->append_output_body_nocopy(content->c_str(), content->size());
                    }
                });

        if(file_idx == file_cnt - 1)
        {
            pread_task->user_data = this; // 设置最后一个文件的 user_data 为当前 HttpResp 对象
        }
        series->push_back(pread_task); // 将文件读取任务添加到任务序列中
        file_idx++;
    }
}

// 压缩数据
int HttpResp::compress(const std::string * const data, std::string *compress_data)
{
    // 初始化状态码为成功
    int status = StatusOK;

    // 检查响应头中是否存在 Content-Encoding 字段
    if (headers.find("Content-Encoding") != headers.end())
    {
        // 如果存在 Content-Encoding 字段且包含 gzip
        if (headers["Content-Encoding"].find("gzip") != std::string::npos)
        {
            // 调用 Compressor::gzip 方法对数据进行 gzip 压缩
            status = Compressor::gzip(data, compress_data);
        }
    } 
    else
    {
        // 如果没有指定 Content-Encoding 字段，设置状态码为未压缩
        status = StatusNoComrpess;
    }

    // 返回压缩状态码
    return status;
}

// 设置错误响应（无错误信息）
void HttpResp::Error(int error_code)
{
    // 调用重载版本，不提供错误信息
    this->Error(error_code, "");
}

// 设置错误响应（带错误信息）
void HttpResp::Error(int error_code, const std::string &errmsg)
{
    // 默认 HTTP 状态码为 503 Service Unavailable
    int status_code = 503;

    // 根据错误代码设置相应的 HTTP 状态码
    switch (error_code)
    {
    case StatusNotFound:
    case StatusRouteVerbNotImplment:
    case StatusRouteNotFound:
        // 如果是路由相关错误，设置状态码为 404 Not Found
        status_code = 404;
        break;
    default:
        break;
    }

    // 设置响应头中的 Content-Type 为 application/json
    this->headers["Content-Type"] = "application/json";

    // 设置 HTTP 状态码
    this->set_status(status_code);

    // 创建 JSON 对象用于存储错误信息
    Yukino::Json js;

    // 获取错误代码对应的错误信息
    std::string resp_msg = error_code_to_str(error_code);

    // 如果提供了额外的错误信息，将其追加到响应消息中
    if (!errmsg.empty()) resp_msg = resp_msg + " : " + errmsg;

    // 如果错误信息是 URL 编码的，进行解码
    if (CodeUtil::is_url_encode(errmsg))
    {
        resp_msg = CodeUtil::url_decode(resp_msg);
    }

    // 将错误信息添加到 JSON 对象中
    js["errmsg"] = resp_msg;

    // 将 JSON 数据作为响应体发送
    this->Json(js);
}

// 设置定时器任务（使用微秒）
void HttpResp::Timer(unsigned int microseconds, const TimerFunc &func)
{
    // 创建定时器任务
    WFTimerTask *timer_task = WFTaskFactory::create_timer_task(microseconds,
                                                               [func](WFTimerTask *) { func(); });

    // 将定时器任务添加到任务列表中
    this->add_task(timer_task);
}

// 设置定时器任务（使用秒和纳秒）
void HttpResp::Timer(time_t seconds, long nanoseconds, const TimerFunc &func)
{
    // 创建定时器任务
    WFTimerTask *timer_task = WFTaskFactory::create_timer_task(seconds, nanoseconds,
                                                               [func](WFTimerTask *) { func(); });

    // 将定时器任务添加到任务列表中
    this->add_task(timer_task);
}

// 推送数据的结构体
struct PushChunkData
{
    std::string data; // 要推送的数据
    size_t nleft = 0; // 剩余未推送的数据量
    HttpServerTask *server_task = nullptr; // 指向 HttpServerTask 的指针
};

// 推送重试回调函数
void push_retry_callback(WFTimerTask *timer_task)
{
    // 从 timer_task 的 user_data 中获取 PushChunkData 对象
    auto* push_chunk_data = static_cast<PushChunkData *>(timer_task->user_data);

    // 获取 HttpServerTask 对象
    auto* server_task = push_chunk_data->server_task;

    // 获取剩余未推送的数据量
    size_t nleft = push_chunk_data->nleft;

    // 计算已推送的数据位置
    size_t pos = push_chunk_data->data.size() - nleft;

    // 尝试推送数据
    size_t nwritten = server_task->push(push_chunk_data->data.c_str() + pos, nleft);

    // 检查推送结果
    if (nwritten >= 0)
    {
        // 如果成功推送了部分或全部数据，更新剩余未推送的数据量
        nleft = nleft - nwritten;
    } 
    else 
    {
        // 如果推送失败
        nwritten = 0;
        if (errno != EWOULDBLOCK)
        {
            // 如果错误不是 EWOULDBLOCK（表示操作会阻塞），则删除 PushChunkData 对象并返回
            delete push_chunk_data;
            return;
        }
    }

    // 如果还有数据未推送
    if (nleft > 0)
    {
        // 更新剩余未推送的数据量
        push_chunk_data->nleft = nleft;

        // 创建一个新的定时器任务，用于重试推送
        timer_task = WFTaskFactory::create_timer_task(0, 1000000, push_retry_callback);
        timer_task->user_data = push_chunk_data;

        // 将定时器任务添加到任务序列的前面
        series_of(server_task)->push_front(timer_task);
    } 
    else 
    {
        // 如果所有数据都已推送完毕，删除 PushChunkData 对象
        delete push_chunk_data;
    }
}

// 推送任务上下文结构体
struct PushTaskCtx
{
    HttpServerTask *server_task = nullptr; // 指向 HttpServerTask 的指针
    std::string cond_name; // 条件名称
    HttpResp::PushFunc push_cb; // 推送回调函数
    HttpResp::PushErrorFunc push_err_cb; // 推送错误回调函数

    // 构造分块数据
    std::string body()
    {
        std::string data;
        // 调用推送回调函数获取数据
        push_cb(data);

        // 构造分块数据
        std::stringstream ss;
        if (!data.empty())
        {
            // 如果有数据，构造分块数据格式
            ss << std::hex << data.size() << "\r\n";
            ss << data << "\r\n";
        }
        else
        {
            // 如果没有数据，表示推送结束
            ss << "0\r\n\r\n";
        }
        return ss.str();
    }
};

// 推送函数
void push_func(WFTimerTask *push_task)
{
    // 从 push_task 的 user_data 中获取 PushTaskCtx 对象
    auto *push_task_ctx = static_cast<PushTaskCtx *>(push_task->user_data);

    // 获取 HttpServerTask 对象
    auto *server_task = push_task_ctx->server_task;

    // 获取 HttpReq 对象
    auto *req = server_task->get_req();

    // 检查是否保持连接或是否已设置关闭标志
    if (!req->is_keep_alive() || server_task->close_flag())
    {
        spdlog::error("[YUKINO] Close the connection");
        return;
    }

    // 构造响应体
    std::string resp_body = push_task_ctx->body();
    size_t nleft = resp_body.size();
    size_t nwritten = server_task->push(resp_body.c_str(), resp_body.size());

    // 检查推送结果
    if (nwritten >= 0)
    {
        nleft = nleft - nwritten;
    } 
    else 
    {
        nwritten = 0;
        if (errno != EWOULDBLOCK)
        {
            // 如果错误不是 EWOULDBLOCK，调用推送错误回调函数
            push_task_ctx->push_err_cb();
            return;
        }
    }

    // 如果还有数据未推送
    if (nleft > 0)
    {
        // 创建 PushChunkData 对象
        auto* push_chunk_data = new PushChunkData;
        push_chunk_data->data = std::move(resp_body);
        push_chunk_data->nleft = nleft;
        push_chunk_data->server_task = server_task;

        // 创建定时器任务用于重试推送
        auto* timer_task = WFTaskFactory::create_timer_task(0, 1000000, push_retry_callback);
        timer_task->user_data = push_chunk_data;

        // 将定时器任务添加到任务序列的前面
        series_of(server_task)->push_front(timer_task);
    }

    // 创建新的定时器任务用于下一次推送
    push_task = WFTaskFactory::create_timer_task(0, 0, push_func);
    push_task->user_data = push_task_ctx;

    // 创建条件任务
    auto *cond = WFTaskFactory::create_conditional(push_task_ctx->cond_name, push_task);

    // 将条件任务添加到服务器任务中
    **server_task << cond;
}

// 构造推送头部
std::string HttpResp::construct_push_header()
{
    std::string http_header;
    http_header.reserve(128);

    // 添加 HTTP 状态行
    http_header.append("HTTP/1.1 200 OK\r\n");

    // 如果存在 Transfer-Encoding 头部，移除它
    if (headers.find("Transfer-Encoding") != headers.end())
    {
        headers.erase("Transfer-Encoding");
    }

    // 遍历所有头部并添加到响应头部
    for (auto it = headers.begin(); it != headers.end(); it++)
    {
        const auto &key = it->first;
        const auto &val = it->second;
        http_header.append(key);
        http_header.append(": ");
        http_header.append(val);
        http_header.append("\r\n");
    }

    // 如果没有 Connection 头部，添加 Connection: close
    if (headers.find("Connection") == headers.end())
    {
        http_header.append("Connection: close\r\n");
    }

    // 添加 Transfer-Encoding: chunked
    http_header.append("Transfer-Encoding: chunked\r\n");

    // 添加空行表示头部结束
    http_header.append("\r\n");

    return http_header;
}

// 推送数据到客户端（仅提供推送回调函数）
void HttpResp::Push(const std::string &cond_name, const PushFunc &push_cb)
{
    // 调用重载版本，提供默认的错误回调函数
    this->Push(cond_name, push_cb, [] {
        spdlog::error("[YUKINO] Connection has lost...");
    });
}

// 推送数据到客户端（提供推送回调函数和错误回调函数）
// 通过定时器和条件任务去推送数据，如果想推送数据，直接发送条件变量即可
// 推送回调函数是用来获取数据的，传入一个字符串参数，然后用户需要给这个字符串添加数据，然后推送函数会自动调用该函数去获取数据
void HttpResp::Push(const std::string &cond_name, const PushFunc &push_cb, const PushErrorFunc &err_cb)
{
    // 获取当前的 HttpServerTask 对象
    HttpServerTask *server_task = task_of(this);

    // 构造 HTTP 响应头部
    std::string http_header = construct_push_header();
    // 将 HTTP 响应头部推送到客户端
    server_task->push(http_header.c_str(), http_header.size());

    // 创建 PushTaskCtx 对象
    auto* push_task_ctx = new PushTaskCtx;
    push_task_ctx->server_task = server_task; // 设置 HttpServerTask 对象
    push_task_ctx->cond_name = cond_name; // 设置条件名称
    push_task_ctx->push_cb = push_cb; // 设置推送回调函数
    push_task_ctx->push_err_cb = err_cb; // 设置错误回调函数

    // 添加回调函数，在任务完成后释放 PushTaskCtx 对象
    server_task->add_callback([push_task_ctx](HttpTask *server_task) {
        delete push_task_ctx;
    });

    // 创建定时器任务用于推送数据
    auto* push_task = WFTaskFactory::create_timer_task(0, 0, push_func);
    push_task->user_data = push_task_ctx; // 设置定时器任务的用户数据

    // 创建条件任务
    auto* cond = WFTaskFactory::create_conditional(cond_name, push_task);

    // 设置服务器任务为无需回复原始响应
    server_task->noreply();
    // 将条件任务添加到服务器任务中
    **server_task << cond;
}

// 发送文件作为响应（仅提供文件路径）
// 如果路径以/开头，则为绝对路径
void HttpResp::File(const std::string &path)
{
    // 调用重载版本，设置起始位置为 0，结束位置为 -1（表示整个文件）
    this->File(path, 0, -1);
}

// 发送文件作为响应（提供文件路径和起始位置）
void HttpResp::File(const std::string &path, size_t start)
{
    // 调用重载版本，设置结束位置为 -1（表示从起始位置到文件末尾）
    this->File(path, start, -1);
}

// 发送文件作为响应（提供文件路径、起始位置和结束位置）
void HttpResp::File(const std::string &path, size_t start, size_t end)
{
    // 调用 HttpFile::send_file 方法发送文件
    int ret = HttpFile::send_file(path, start, end, this);
    // 如果发送失败，设置错误响应
    if (ret != StatusOK)
    {
        this->Error(ret);
    }
}

// 设置 HTTP 响应状态码
void HttpResp::set_status(int status_code)
{
    // 调用 protocol::HttpUtil::set_response_status 方法设置状态码
    protocol::HttpUtil::set_response_status(this, status_code);
}

// 保存内容到文件（左值引用）
void HttpResp::Save(const std::string &file_dst, const std::string &content)
{
    // 调用 HttpFile::save_file 方法保存内容到文件
    HttpFile::save_file(file_dst, content, this);
}

// 保存内容到文件（右值引用优化）
void HttpResp::Save(const std::string &file_dst, std::string &&content)
{
    // 调用 HttpFile::save_file 方法保存内容到文件，使用右值引用优化性能
    HttpFile::save_file(file_dst, std::move(content), this);
}

// 保存内容到文件（左值引用，带通知消息）
void HttpResp::Save(const std::string &file_dst, const std::string &content, const std::string &notify_msg)
{
    // 调用 HttpFile::save_file 方法保存内容到文件，并提供通知消息
    HttpFile::save_file(file_dst, content, this, notify_msg);
}

// 保存内容到文件（右值引用优化，带通知消息）
void HttpResp::Save(const std::string &file_dst, std::string &&content, const std::string &notify_msg)
{
    // 调用 HttpFile::save_file 方法保存内容到文件，使用右值引用优化性能，并提供通知消息
    HttpFile::save_file(file_dst, std::move(content), this, notify_msg);
}

// 保存内容到文件（左值引用，带文件 I/O 回调函数）
void HttpResp::Save(const std::string &file_dst, const std::string &content,
        const HttpFile::FileIOArgsFunc &func)
{
    // 调用 HttpFile::save_file 方法保存内容到文件，并提供文件 I/O 回调函数
    HttpFile::save_file(file_dst, content, this, func);
}

// 保存内容到文件（右值引用优化，带文件 I/O 回调函数）
void HttpResp::Save(const std::string &file_dst, std::string &&content,
        const HttpFile::FileIOArgsFunc &func)
{
    // 调用 HttpFile::save_file 方法保存内容到文件，使用右值引用优化性能，并提供文件 I/O 回调函数
    HttpFile::save_file(file_dst, std::move(content), this, func);
}

// 设置 JSON 响应（使用 Yukino::Json 对象）
void HttpResp::Json(const Yukino::Json &json)
{
    // 设置响应头中的 Content-Type 为 application/json
    this->headers["Content-Type"] = "application/json";

    // 将 JSON 对象序列化为字符串并添加到响应体中
    this->String(json.dump());
}

// 设置 JSON 响应（使用字符串）
void HttpResp::Json(const std::string &str)
{
    // 尝试解析字符串为 JSON 对象
    if (!Yukino::Json::parse(str).is_valid())
    {
        // 如果解析失败，设置错误响应
        this->Error(StatusJsonInvalid);
        return;
    }

    // 设置响应头中的 Content-Type 为 application/json
    this->headers["Content-Type"] = "application/json";

    // 将字符串添加到响应体中
    this->String(str);
}

// 设置响应的压缩方式
void HttpResp::set_compress(const enum Compress &compress)
{
    // 根据提供的压缩方式设置 Content-Encoding 头部
    headers["Content-Encoding"] = compress_method_to_str(compress);
}

// 获取当前任务的状态
int HttpResp::get_state() const
{
    // 获取当前的 HttpServerTask 对象
    HttpServerTask *server_task = task_of(this);
    // 返回任务的状态
    return server_task->get_state();
}

// 获取当前任务的错误码
int HttpResp::get_error() const
{
    // 获取当前的 HttpServerTask 对象
    HttpServerTask *server_task = task_of(this);
    // 返回任务的错误码
    return server_task->get_error();
}

// 发起 HTTP 请求
void HttpResp::Http(const std::string &url, int redirect_max, size_t size_limit)
{
    // 获取当前的 HttpServerTask 对象
    HttpServerTask *server_task = task_of(this);
    // 获取当前的 HttpReq 对象
    HttpReq *server_req = server_task->get_req();
    // 构造完整的 HTTP URL
    std::string http_url = url;
    if (strncasecmp(url.c_str(), "http://", 7) != 0 &&
        strncasecmp(url.c_str(), "https://", 8) != 0)
    {
        // 如果 URL 不包含协议头，则默认添加 "http://"
        http_url = "http://" + http_url;
    }

    // 创建 HTTP 任务
    WFHttpTask *http_task = WFTaskFactory::create_http_task(http_url,
                                                            redirect_max,
                                                            0,
                                                            proxy_http_callback);

    // 创建代理上下文
    auto *proxy_ctx = new ProxyCtx;
    proxy_ctx->url = http_url; // 设置目标 URL
    proxy_ctx->server_task = server_task; // 设置服务器任务
    proxy_ctx->is_keep_alive = server_req->is_keep_alive(); // 设置是否保持连接

    // 设置 HTTP 任务的用户数据为代理上下文
    http_task->user_data = proxy_ctx;
    
    // 下面这几步都是获取客户端的路径参数等信息，然后拷贝到服务器的http请求中，模仿真实客户端去访问目标服务器
    // 获取请求体
    const void *body;
    size_t len;

    // 解析 URL
    ParsedURI uri;
    if (URIParser::parse(http_url, uri) < 0)
    {
        // 如果解析失败，设置状态码为 400 Bad Request 并返回
        server_task->get_resp()->set_status(HttpStatusBadRequest);
        return;
    }

    // 构造路由路径
    std::string route;
    if (uri.path && uri.path[0])
    {
        route.append(uri.path); // 添加路径
    } 
    else
    {
        route.append("/"); // 如果没有路径，默认为根路径
    }

    if (uri.query && uri.query[0])
    {
        route.append("?"); // 添加查询字符串
        route.append(uri.query);
    }

    // 注意这里为了提高性能，采取了移动语义，直接将客户端的request复用到服务器的request中，然后在代理中的回调函数中再转回来。
    // 设置请求的 URI
    server_req->set_request_uri(route);

    // 获取请求体
    server_req->get_parsed_body(&body, &len);
    // 将请求体添加到输出体中
    server_req->append_output_body_nocopy(body, len);

    // 将 HttpReq 对象的特定部分移动到 WFHttpTask 的请求对象中
    HttpRequest *server_req_cast = static_cast<HttpRequest *>(server_req);
    *http_task->get_req() = std::move(*server_req_cast);

    // 设置响应体大小限制
    http_task->get_resp()->set_size_limit(size_limit);

    // 将 HTTP 任务添加到服务器任务中
    **server_task << http_task;
}

// 执行 MySQL 查询（不带回调函数）
void HttpResp::MySQL(const std::string &url, const std::string &sql)
{
    // 创建 MySQL 任务
    WFMySQLTask *mysql_task = WFTaskFactory::create_mysql_task(url, 0, mysql_callback);
    // 设置查询语句
    mysql_task->get_req()->set_query(sql);
    // 设置任务的用户数据为当前 HttpResp 对象
    mysql_task->user_data = this;
    // 将 MySQL 任务添加到任务列表中
    this->add_task(mysql_task);
}

// 执行 MySQL 查询（带 JSON 回调函数）
void HttpResp::MySQL(const std::string &url, const std::string &sql, const MySQLJsonFunc &func)
{
    // 创建 MySQL 任务，使用 lambda 表达式作为回调函数
    WFMySQLTask *mysql_task = WFTaskFactory::create_mysql_task(url, 0,
    [func](WFMySQLTask *mysql_task)
    {
        // 将 MySQL 查询结果转换为 JSON 格式
        Yukino::Json json = mysql_concat_json_res(mysql_task);
        // 调用用户提供的回调函数
        func(&json);
    });

    // 设置查询语句
    mysql_task->get_req()->set_query(sql);
    // 将 MySQL 任务添加到任务列表中
    this->add_task(mysql_task);
}

// 执行 MySQL 查询（带自定义回调函数）
void HttpResp::MySQL(const std::string &url, const std::string &sql, const MySQLFunc &func)
{
    // 创建 MySQL 任务，使用 lambda 表达式作为回调函数
    WFMySQLTask *mysql_task = WFTaskFactory::create_mysql_task(url, 0,
    [func](WFMySQLTask *mysql_task)
    {
        // 检查任务状态
        if (mysql_task->get_state() != WFT_STATE_SUCCESS)
        {
            // 如果任务失败，生成错误信息并设置响应体
            std::string errmsg = WFGlobal::get_error_string(mysql_task->get_state(),
                                                mysql_task->get_error());
            auto *server_resp = static_cast<HttpResp *>(mysql_task->user_data);
            server_resp->String(std::move(errmsg));
            return;
        }

        // 获取 MySQL 响应对象并创建结果游标
        MySQLResponse *mysql_resp = mysql_task->get_resp();
        MySQLResultCursor cursor(mysql_resp);

        // 调用用户提供的回调函数
        func(&cursor);
    });

    // 设置查询语句
    mysql_task->get_req()->set_query(sql);
    // 设置任务的用户数据为当前 HttpResp 对象
    mysql_task->user_data = this;
    // 将 MySQL 任务添加到任务列表中
    this->add_task(mysql_task);
}

// 执行 Redis 命令
void HttpResp::Redis(const std::string &url, const std::string &command,
        const std::vector<std::string>& params)
{
    // 创建 Redis 任务，使用 lambda 表达式作为回调函数
    WFRedisTask *redis_task = WFTaskFactory::create_redis_task(url, 2, [this](WFRedisTask *redis_task)
    {
        // 将 Redis 响应结果转换为 JSON 格式
        Yukino::Json js = redis_json_res(redis_task);
        // 设置 JSON 数据作为响应体
        this->Json(js);
    });

    // 设置 Redis 命令和参数
    redis_task->get_req()->set_request(command, params);
    // 将 Redis 任务添加到任务列表中
    this->add_task(redis_task);
}

// 执行 Redis 命令（带 JSON 回调函数）
void HttpResp::Redis(const std::string &url, const std::string &command,
    const std::vector<std::string>& params, const RedisJsonFunc &func)
{
    // 创建 Redis 任务，使用 lambda 表达式作为回调函数
    WFRedisTask *redis_task = WFTaskFactory::create_redis_task(url, 2, [func](WFRedisTask *redis_task)
    {
        // 将 Redis 响应结果转换为 JSON 格式
        Yukino::Json js = redis_json_res(redis_task);
        // 调用用户提供的 JSON 回调函数
        func(&js);
    });

    // 设置 Redis 命令和参数
    redis_task->get_req()->set_request(command, params);

    // 将 Redis 任务添加到任务列表中
    this->add_task(redis_task);
}

// 执行 Redis 命令（带自定义回调函数）
void HttpResp::Redis(const std::string &url, const std::string &command,
    const std::vector<std::string>& params, const RedisFunc &func)
{
    // 创建 Redis 任务，使用用户提供的回调函数
    WFRedisTask *redis_task = WFTaskFactory::create_redis_task(url, 2, func);

    // 设置 Redis 命令和参数
    redis_task->get_req()->set_request(command, params);

    // 将 Redis 任务添加到任务列表中
    this->add_task(redis_task);
}

// 发起重定向响应
void HttpResp::Redirect(const std::string& location, int status_code)
{
    // 设置 Location 响应头
    this->headers["Location"] = location;

    // 设置 HTTP 状态码
    this->set_status(status_code);
}

// 添加子任务到任务列表
void HttpResp::add_task(SubTask *task)
{
    // 获取当前的 HttpServerTask 对象
    HttpServerTask *server_task = task_of(this);

    // 将子任务添加到服务器任务中
    **server_task << task;
}

// HttpResp 的移动构造函数
HttpResp::HttpResp(HttpResp&& other)
: HttpResponse(std::move(other)), // 调用基类的移动构造函数
headers(std::move(other.headers)), // 移动响应头映射
cookies_(std::move(other.cookies_)) // 移动 Cookie 列表
{
    user_data = other.user_data; // 移动用户数据指针
    other.user_data = nullptr; // 将源对象的用户数据指针置空
}

// HttpResp 的移动赋值运算符
HttpResp &HttpResp::operator=(HttpResp&& other)
{
    // 调用基类的移动赋值运算符
    HttpResponse::operator=(std::move(other));

    // 移动响应头映射
    headers = std::move(other.headers);

    // 移动用户数据指针
    user_data = other.user_data;
    other.user_data = nullptr;

    // 移动 Cookie 列表
    cookies_ = std::move(other.cookies_);

    // 返回当前对象的引用
    return *this;
}

} // namespace Yukino

