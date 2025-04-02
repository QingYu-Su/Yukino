#ifndef YUKINO_HTTPDEF_H_
#define YUKINO_HTTPDEF_H_

#include <string>

namespace Yukino
{
    // MIME 类型定义，参考 IANA 官方文档：https://www.iana.org/assignments/media-types/media-types.xhtml
    // 使用宏定义列出常见的 HTTP 内容类型及其对应的 MIME 类型和文件后缀
    // XX(name, mime, suffix)
    #define HTTP_CONTENT_TYPE_MAP(XX) \
        XX(TEXT_PLAIN,              text/plain,               txt)          \
        XX(TEXT_HTML,               text/html,                html)         \
        XX(TEXT_CSS,                text/css,                 css)          \
        XX(IMAGE_JPEG,              image/jpeg,               jpg)          \
        XX(IMAGE_PNG,               image/png,                png)          \
        XX(IMAGE_GIF,               image/gif,                gif)          \
        XX(IMAGE_BMP,               image/bmp,                bmp)          \
        XX(IMAGE_SVG,               image/svg,                svg)          \
        XX(APPLICATION_OCTET_STREAM,application/octet-stream, bin)          \
        XX(APPLICATION_JAVASCRIPT,  application/javascript,   js)           \
        XX(APPLICATION_XML,         application/xml,          xml)          \
        XX(APPLICATION_JSON,        application/json,         json)         \
        XX(APPLICATION_GRPC,        application/grpc,         grpc)         \
        XX(APPLICATION_URLENCODED,  application/x-www-form-urlencoded, kv)  \
        XX(MULTIPART_FORM_DATA,     multipart/form-data,               mp)  \

    // 为了兼容性，定义一个别名
    #define X_WWW_FORM_URLENCODED   APPLICATION_URLENCODED // for compatibility

    // 定义一个枚举类型，用于表示 HTTP 内容类型
    enum http_content_type
    {
        // 使用宏定义生成枚举值
        #define XX(name, string, suffix)   name,
        CONTENT_TYPE_NONE,  // 无内容类型
        HTTP_CONTENT_TYPE_MAP(XX)  // 使用宏展开生成枚举值
        CONTENT_TYPE_UNDEFINED  // 未定义的内容类型
        #undef XX
    };

    // 定义一个 ContentType 类，用于处理 HTTP 内容类型
    class ContentType
    {
    public:
        // 将枚举类型转换为字符串
        static std::string to_str(enum http_content_type type);

        // 根据文件后缀获取 MIME 类型字符串
        static std::string to_str_by_suffix(const std::string &suffix);

        // 将 MIME 类型字符串转换为枚举类型
        static enum http_content_type to_enum(const std::string &content_type_str);

        // 根据文件后缀获取对应的枚举类型
        static enum http_content_type to_enum_by_suffix(const std::string &suffix);
    };

} // namespace Yukino

#endif // YUKINO_HTTPDEF_H_