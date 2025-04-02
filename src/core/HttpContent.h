#ifndef YUKINO_HTTPCONTENT_H_
#define YUKINO_HTTPCONTENT_H_

#include <string>
#include <map>
#include <vector>
#include "MultiPartParser.h"
#include "Noncopyable.h"

namespace Yukino
{

class StringPiece;

/**
 * @brief 用于解析 URL 编码的 POST 请求体。
 */
class Urlencode
{
public:
    /**
     * @brief 解析 URL 编码的 POST 请求体为键值对。
     *
     * @param body 请求体内容。
     * @return std::map<std::string, std::string> 解析后的键值对。
     */
    static std::map<std::string, std::string> parse_post_kv(const StringPiece &body);
};

// <url id="cvgm8ugh8nju4lqmh7o0" type="url" status="parsed" title="POST - HTTP | MDN" wc="4981">https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods/POST</url> 
// <name ,<filename, body>>
using Form = std::map<std::string, std::pair<std::string, std::string>>;

/**
 * @brief 用于解析多部分表单数据（multipart/form-data）。
 */
class MultiPartForm 
{
public:
    /**
     * @brief 解析多部分表单数据。
     *
     * @param body 请求体内容。
     * @return Form 解析后的表单数据。
     */
    Form parse_multipart(const StringPiece &body) const;

    /**
     * @brief 设置多部分表单的边界字符串。
     *
     * @param boundary 边界字符串。
     */
    void set_boundary(std::string &&boundary)
    { boundary_ = std::move(boundary); }

    /**
     * @brief 设置多部分表单的边界字符串。
     *
     * @param boundary 边界字符串。
     */
    void set_boundary(const std::string &boundary)
    { boundary_ = boundary; }

public:
    /**
     * @brief 默认的边界字符串。
     */
    static const std::string k_default_boundary;

    /**
     * @brief 构造函数。
     */
    MultiPartForm();

private:
    /**
     * @brief 回调函数，处理头部字段。
     *
     * @param parser 解析器。
     * @param buf 数据缓冲区。
     * @param len 数据长度。
     * @return int 回调结果。
     */
    static int header_field_cb(multipart_parser *parser, const char *buf, size_t len);

    /**
     * @brief 回调函数，处理头部值。
     *
     * @param parser 解析器。
     * @param buf 数据缓冲区。
     * @param len 数据长度。
     * @return int 回调结果。
     */
    static int header_value_cb(multipart_parser *parser, const char *buf, size_t len);

    /**
     * @brief 回调函数，处理部分数据。
     *
     * @param parser 解析器。
     * @param buf 数据缓冲区。
     * @param len 数据长度。
     * @return int 回调结果。
     */
    static int part_data_cb(multipart_parser *parser, const char *buf, size_t len);

    /**
     * @brief 回调函数，处理部分数据开始。
     *
     * @param parser 解析器。
     * @return int 回调结果。
     */
    static int part_data_begin_cb(multipart_parser *parser);

    /**
     * @brief 回调函数，处理头部完成。
     *
     * @param parser 解析器。
     * @return int 回调结果。
     */
    static int headers_complete_cb(multipart_parser *parser);

    /**
     * @brief 回调函数，处理部分数据结束。
     *
     * @param parser 解析器。
     * @return int 回调结果。
     */
    static int part_data_end_cb(multipart_parser *parser);

    /**
     * @brief 回调函数，处理请求体结束。
     *
     * @param parser 解析器。
     * @return int 回调结果。
     */
    static int body_end_cb(multipart_parser *parser);

private:
    std::string boundary_;  // 边界字符串
    multipart_parser_settings settings_;  // 解析器设置
};

/**
 * @brief 用于生成多部分表单数据（multipart/form-data）。
 */
class MultiPartEncoder 
{
public:
    /**
     * @brief 参数列表类型。
     */
    using ParamList = std::vector<std::pair<std::string, std::string>>;

    /**
     * @brief 文件列表类型。
     */
    using FileList = std::vector<std::pair<std::string, std::string>>;

    /**
     * @brief 构造函数。
     */
    MultiPartEncoder();

    /**
     * @brief 析构函数。
     */
    ~MultiPartEncoder() = default;

    /**
     * @brief 添加表单参数。
     *
     * @param name 参数名。
     * @param value 参数值。
     */
    void add_param(const std::string &name, const std::string &value);

    /**
     * @brief 添加文件。
     *
     * @param file_name 文件名。
     * @param file_path 文件路径。
     */
    void add_file(const std::string &file_name, const std::string &file_path);

    /**
     * @brief 获取参数列表。
     *
     * @return const ParamList& 参数列表。
     */
    const ParamList &params() const { return params_; }

    /**
     * @brief 获取文件列表。
     *
     * @return const FileList& 文件列表。
     */
    const FileList &files() const { return files_; }

    /**
     * @brief 获取边界字符串。
     *
     * @return const std::string& 边界字符串。
     */
    const std::string &boundary() const { return boundary_; }

    /**
     * @brief 设置边界字符串。
     *
     * @param boundary 边界字符串。
     */
    void set_boundary(const std::string &boundary);

    /**
     * @brief 设置边界字符串（移动语义）。
     *
     * @param boundary 边界字符串。
     */
    void set_boundary(std::string &&boundary);

private:
    std::string boundary_;  // 边界字符串
    std::string content_;   // 请求体内容
    ParamList params_;      // 参数列表
    FileList files_;        // 文件列表
};

}  // namespace Yukino

#endif // YUKINO_HTTPCONTENT_H_