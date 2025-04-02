#ifndef YUKINO_HTTPCOOKIE_H_
#define YUKINO_HTTPCOOKIE_H_

#include <string>
#include <map>
#include "Timestamp.h"
#include "StringPiece.h"
#include "Copyable.h"

namespace Yukino
{
/**
 * @brief 枚举类，表示 SameSite 属性的值。
 *
 * SameSite 属性控制 cookie 是否随跨站请求发送，以防止某些跨站攻击（如 CSRF）。
 */
enum class SameSite
{
    DEFAULT,  // 默认值，未明确指定时的行为
    STRICT,   // 严格模式，仅在同站请求中发送 cookie
    LAX,      // 宽松模式，允许在某些跨站请求中发送 cookie
    NONE      // 无限制，允许在所有请求中发送 cookie，但必须配合 Secure 属性
};

/**
 * @brief 将 SameSite 枚举值转换为字符串。
 *
 * @param same_site SameSite 枚举值。
 * @return const char* 对应的字符串。
 */
inline const char *same_site_to_str(SameSite same_site)
{
    switch (same_site)
    {
        case SameSite::STRICT:
            return "Strict";
        case SameSite::LAX:
            return "Lax";
        case SameSite::NONE:
            return "None";
        default:
            return "";
    }
}

class HttpCookie : public Copyable
{
public:
    /**
     * @brief 检查 Cookie 是否为空。
     *
     * 如果 key 和 value 都不为空，则认为 Cookie 是有效的。
     *
     * @return bool 如果 Cookie 有效，返回 true；否则返回 false。
     */
    explicit operator bool() const
    { return (!key_.empty()) && (!value_.empty()); }

    /**
     * @brief 将 Cookie 的内容转换为字符串。
     *
     * @return std::string Cookie 的字符串表示。
     */
    std::string dump() const;

    /**
     * @brief 将 Cookie 字符串分割为键值对。
     *
     * @param cookie_piece 包含 Cookie 内容的 StringPiece。
     * @return std::map<std::string, std::string> 键值对映射。
     */
    static std::map<std::string, std::string> split(const StringPiece &cookie_piece);

public:
    /**
     * @brief 设置 Cookie 的 key。
     *
     * @param key Cookie 的 key。
     * @return HttpCookie& 当前对象的引用。
     */
    HttpCookie &set_key(const std::string &key)
    {
        key_ = key;
        return *this;
    }

    /**
     * @brief 设置 Cookie 的 key（移动语义）。
     *
     * @param key Cookie 的 key。
     * @return HttpCookie& 当前对象的引用。
     */
    HttpCookie &set_key(std::string &&key)
    {
        key_ = std::move(key);
        return *this;
    }

    /**
     * @brief 设置 Cookie 的 value。
     *
     * @param value Cookie 的 value。
     * @return HttpCookie& 当前对象的引用。
     */
    HttpCookie &set_value(const std::string &value)
    {
        value_ = value;
        return *this;
    }

    /**
     * @brief 设置 Cookie 的 value（移动语义）。
     *
     * @param value Cookie 的 value。
     * @return HttpCookie& 当前对象的引用。
     */
    HttpCookie &set_value(std::string &&value)
    {
        value_ = std::move(value);
        return *this;
    }

    /**
     * @brief 设置 Cookie 的 domain。
     *
     * @param domain Cookie 的 domain。
     * @return HttpCookie& 当前对象的引用。
     */
    HttpCookie &set_domain(const std::string &domain)
    {
        domain_ = domain;
        return *this;
    }

    /**
     * @brief 设置 Cookie 的 domain（移动语义）。
     *
     * @param domain Cookie 的 domain。
     * @return HttpCookie& 当前对象的引用。
     */
    HttpCookie &set_domain(std::string &&domain)
    {
        domain_ = std::move(domain);
        return *this;
    }

    /**
     * @brief 设置 Cookie 的 path。
     *
     * @param path Cookie 的 path。
     * @return HttpCookie& 当前对象的引用。
     */
    HttpCookie &set_path(const std::string &path)
    {
        path_ = path;
        return *this;
    }

    /**
     * @brief 设置 Cookie 的 path（移动语义）。
     *
     * @param path Cookie 的 path。
     * @return HttpCookie& 当前对象的引用。
     */
    HttpCookie &set_path(std::string &&path)
    {
        path_ = std::move(path);
        return *this;
    }

    /**
     * @brief 设置 Cookie 的过期时间。
     *
     * @param expires Cookie 的过期时间。
     * @return HttpCookie& 当前对象的引用。
     */
    HttpCookie &set_expires(const Timestamp &expires)
    {
        expires_ = expires;
        return *this;
    }

    /**
     * @brief 设置 Cookie 的最大生存时间（秒）。
     *
     * @param max_age Cookie 的最大生存时间（秒）。
     * @return HttpCookie& 当前对象的引用。
     */
    HttpCookie &set_max_age(int max_age)
    {
        max_age_ = max_age;
        return *this;
    }

    /**
     * @brief 设置 Cookie 的 Secure 属性。
     *
     * @param secure 是否启用 Secure 属性。
     * @return HttpCookie& 当前对象的引用。
     */
    HttpCookie &set_secure(bool secure)
    {
        secure_ = secure;
        return *this;
    }

    /**
     * @brief 设置 Cookie 的 HttpOnly 属性。
     *
     * @param http_only 是否启用 HttpOnly 属性。
     * @return HttpCookie& 当前对象的引用。
     */
    HttpCookie &set_http_only(bool http_only)
    {
        http_only_ = http_only;
        return *this;
    }

    /**
     * @brief 设置 Cookie 的 SameSite 属性。
     *
     * @param same_site SameSite 属性值。
     * @return HttpCookie& 当前对象的引用。
     */
    HttpCookie &set_same_site(SameSite same_site)
    {
        same_site_ = same_site;
        return *this;
    }

    public:
    /**
     * @brief 获取 Cookie 的 key。
     *
     * @return const std::string& Cookie 的 key。
     */
    const std::string &key() const
    { return key_; }

    /**
     * @brief 获取 Cookie 的 value。
     *
     * @return const std::string& Cookie 的 value。
     */
    const std::string &value() const
    { return value_; }

    /**
     * @brief 获取 Cookie 的 domain。
     *
     * @return const std::string& Cookie 的 domain。
     */
    const std::string &domain() const
    { return domain_; }

    /**
     * @brief 获取 Cookie 的 path。
     *
     * @return const std::string& Cookie 的 path。
     */
    const std::string &path() const
    { return path_; }

    /**
     * @brief 获取 Cookie 的过期时间。
     *
     * @return Timestamp Cookie 的过期时间。
     */
    Timestamp expires() const
    { return expires_; }

    /**
     * @brief 获取 Cookie 的最大生存时间（秒）。
     *
     * @return int Cookie 的最大生存时间（秒）。
     */
    int max_age() const
    { return max_age_; }

    /**
     * @brief 检查 Cookie 是否启用了 Secure 属性。
     *
     * @return bool 如果启用了 Secure 属性，则返回 true；否则返回 false。
     */
    bool is_secure() const
    { return secure_; }

    /**
     * @brief 检查 Cookie 是否启用了 HttpOnly 属性。
     *
     * @return bool 如果启用了 HttpOnly 属性，则返回 true；否则返回 false。
     */
    bool is_http_only() const
    { return http_only_; }

    /**
     * @brief 获取 Cookie 的 SameSite 属性。
     *
     * @return SameSite Cookie 的 SameSite 属性值。
     */
    SameSite same_site() const
    { return same_site_; }

public:
    /**
     * @brief 构造函数，初始化 Cookie 的 key 和 value。
     *
     * @param key Cookie 的 key。
     * @param value Cookie 的 value。
     */
    HttpCookie(const std::string &key, const std::string &value)
            : key_(key), value_(value)
    {}

    /**
     * @brief 构造函数，初始化 Cookie 的 key 和 value（移动语义）。
     *
     * @param key Cookie 的 key。
     * @param value Cookie 的 value。
     */
    HttpCookie(std::string &&key, std::string &&value)
            : key_(std::move(key)), value_(std::move(value))
    {}

    /**
     * @brief 默认构造函数。
     */
    HttpCookie() = default;

private:
    std::string key_;          // Cookie 的 key
    std::string value_;        // Cookie 的 value
    std::string domain_;       // Cookie 的 domain
    std::string path_;         // Cookie 的 path

    Timestamp expires_;        // Cookie 的过期时间
    int max_age_ = 0;          // Cookie 的最大生存时间（秒）
    bool secure_ = false;      // 是否启用 Secure 属性
    bool http_only_ = false;   // 是否启用 HttpOnly 属性

    SameSite same_site_ = SameSite::DEFAULT;  // Cookie 的 SameSite 属性
};

} // namespace Yukino




#endif // YUKINO_HTTPCOOKIE_H_