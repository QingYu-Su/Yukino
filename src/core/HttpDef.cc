#include "HttpDef.h"
#include <cstring>

using namespace Yukino;

namespace 
{
    /**
     * @brief 检查字符串是否以指定的前缀开头。
     *
     * @param str 要检查的字符串。
     * @param start 前缀字符串。
     * @return int 如果 str 以 start 开头，返回 1；否则返回 0。
     */
    int strstartswith(const char *str, const char *start)
    {
        while (*str && *start && *str == *start)
        {
            ++str;
            ++start;
        }
        return *start == '\0';
    }
} // namespace 

/**
 * @brief 将 HTTP 内容类型枚举值转换为对应的 MIME 类型字符串。
 *
 * @param type 要转换的 HTTP 内容类型枚举值。
 * @return std::string 对应的 MIME 类型字符串，如果类型未知，则返回 "<unknown>"。
 */
std::string ContentType::to_str(enum http_content_type type)
{
    switch (type)
    {
#define XX(name, string, suffix) case name: return #string;
        HTTP_CONTENT_TYPE_MAP(XX)
#undef XX
        default:
            return "<unknown>";
    }
}

/**
 * @brief 将 MIME 类型字符串转换为对应的 HTTP 内容类型枚举值。
 *
 * @param content_type_str 要转换的 MIME 类型字符串。
 * @return enum http_content_type 对应的 HTTP 内容类型枚举值，如果字符串为空，则返回 CONTENT_TYPE_NONE；如果类型未知，则返回 CONTENT_TYPE_UNDEFINED。
 */
enum http_content_type ContentType::to_enum(const std::string &content_type_str)
{
    if (content_type_str.empty())
    {
        return CONTENT_TYPE_NONE;
    }
#define XX(name, string, suffix) \
    if (strstartswith(content_type_str.c_str(), #string)) { \
        return name; \
    }
    HTTP_CONTENT_TYPE_MAP(XX)
#undef XX
    return CONTENT_TYPE_UNDEFINED;
}

/**
 * @brief 根据文件后缀获取对应的 MIME 类型字符串。
 *
 * @param str 文件后缀。
 * @return std::string 对应的 MIME 类型字符串，如果后缀为空或未知，则返回空字符串。
 */
std::string ContentType::to_str_by_suffix(const std::string &str)
{
    if (str.empty())
    {
        return "";
    }
#define XX(name, string, suffix) \
    if (str == #suffix) { \
        return #string; \
    }
    HTTP_CONTENT_TYPE_MAP(XX)
#undef XX
    return "";
}

/**
 * @brief 根据文件后缀获取对应的 HTTP 内容类型枚举值。
 *
 * @param str 文件后缀。
 * @return enum http_content_type 对应的 HTTP 内容类型枚举值，如果后缀为空，则返回 CONTENT_TYPE_NONE；如果后缀未知，则返回 CONTENT_TYPE_UNDEFINED。
 */
enum http_content_type ContentType::to_enum_by_suffix(const std::string &str)
{
    if (str.empty()) {
        return CONTENT_TYPE_NONE;
    }
#define XX(name, string, suffix) \
    if (str == #suffix) { \
        return name; \
    }
    HTTP_CONTENT_TYPE_MAP(XX)
#undef XX
    return CONTENT_TYPE_UNDEFINED;
}