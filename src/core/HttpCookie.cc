#include <vector>
#include "HttpCookie.h"
#include "StrUtil.h"

using namespace Yukino;

std::map<std::string, std::string> HttpCookie::split(const StringPiece &cookie_piece)
{
    std::map<std::string, std::string> res;

    // 如果输入的 Cookie 字符串为空，直接返回空的 map
    if (cookie_piece.empty())
        return res;

    // 使用逗号分隔 Cookie 字符串，例如 "user=Yukino, passwd=123" 分割为 ["user=Yukino", " passwd=123"]
    std::vector<StringPiece> arr = StrUtil::split_piece<StringPiece>(cookie_piece, ',');

    // 如果分割结果为空，直接返回空的 map
    if (arr.empty())
        return res;

    // 临时存储分割后的键值对
    std::map<StringPiece, StringPiece> piece_res;

    // 遍历分割后的每个元素
    for (const auto &ele : arr)
    {
        // 如果当前元素为空，跳过
        if (ele.empty())
            continue;

        // 使用等号分隔键值对，例如 "user=Yukino" 分割为 ["user", "Yukino"]
        std::vector<StringPiece> kv = StrUtil::split_piece<StringPiece>(ele, '=');
        size_t kv_size = kv.size();
        StringPiece &key = kv[0];

        // 如果键为空或键已存在于 map 中，跳过
        if (key.empty() || piece_res.count(key) > 0)
            continue;

        // 如果只有键没有值，将键值对插入 map，值为空字符串
        if (kv_size == 1)
        {
            piece_res.emplace(StrUtil::trim(key), "");
            continue;
        }

        // 获取值
        StringPiece &val = kv[1];

        // 如果值为空，将键值对插入 map，值为空字符串
        if (val.empty())
            piece_res.emplace(StrUtil::trim(key), "");
        else
            // 将键值对插入 map，键和值都去掉首尾空格
            piece_res.emplace(StrUtil::trim(key), StrUtil::trim(val));
    }

    // 将临时存储的键值对转换为最终的 map
    for (auto &piece : piece_res)
    {
        res.emplace(piece.first.as_string(), piece.second.as_string());
    }

    return res;
}

std::string HttpCookie::dump() const
{
    std::string ret;
    ret.reserve(key_.size() + value_.size() + 30);  // 预分配内存以提高性能
    ret.append(key_).append("=").append(value_).append("; ");  // 添加 key 和 value

    // 如果同时设置了 Max-Age 和 Expires，Max-Age 优先
    bool has_max_age = max_age_ > 0;
    if (has_max_age)
    {
        ret.append("Max-Age").append("=").append(std::to_string(max_age_)).append("; ");  // 添加 Max-Age
    }
    if (!has_max_age && expires_.valid())  // 如果没有设置 Max-Age 但设置了 Expires
    {
        ret.append("Expires=")
                .append(expires_.to_format_str("%a, %d %b %Y %H:%M:%S GMT"))  // 格式化日期
                .append("; ");
    }
    if (!domain_.empty())  // 如果设置了 Domain
    {
        ret.append("Domain=").append(domain_).append("; ");
    }
    if (!path_.empty())  // 如果设置了 Path
    {
        ret.append("Path=").append(path_).append("; ");
    }
    if (secure_)  // 如果设置了 Secure 属性
    {
        ret.append("Secure; ");
    }
    if (http_only_)  // 如果设置了 HttpOnly 属性
    {
        ret.append("HttpOnly; ");
    }
    std::string same_site = same_site_to_str(same_site_);  // 将 SameSite 枚举值转换为字符串
    if (!same_site.empty())  // 如果设置了 SameSite
    {
        ret.append("SameSite=").append(same_site).append("; ");
    }
    // 如果 SameSite=None 且没有设置 Secure 属性，则必须添加 Secure
    if (same_site == "None" && !secure_)
    {
        ret.append("Secure; ");
    } 
    ret.resize(ret.length() - 2);  // 去掉多余的分号和空格
    return ret;
}
