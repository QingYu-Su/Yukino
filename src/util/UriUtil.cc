#include "UriUtil.h"
#include "StrUtil.h"

using namespace Yukino;

std::map<std::string, std::string> UriUtil::split_query(const StringPiece &query)
{
    std::map<std::string, std::string> res; // 用于存储解析后的键值对

    // 如果查询字符串为空，直接返回空映射表
    if (query.empty())
        return res;

    // 使用 StrUtil::split_piece 将查询字符串按 '&' 分割为多个子字符串
    std::vector<StringPiece> arr = StrUtil::split_piece<StringPiece>(query, '&');

    // 如果分割后没有子字符串，返回空映射表
    if (arr.empty())
        return res;

    // 遍历每个子字符串（每个子字符串代表一个键值对）
    for (const auto &ele : arr)
    {
        // 如果子字符串为空，跳过
        if (ele.empty())
            continue;

        // 使用 StrUtil::split_piece 将子字符串按 '=' 分割为键和值
        std::vector<std::string> kv = StrUtil::split_piece<std::string>(ele, '=');
        size_t kv_size = kv.size(); // 获取键值对的数量
        std::string &key = kv[0]; // 获取键

        // 如果键为空，或者键已经存在于结果映射表中，跳过
        if (key.empty() || res.count(key) > 0)
            continue;

        // 如果键值对只有一个元素（即没有值），将键添加到映射表中，值为空字符串
        if (kv_size == 1)
        {
            res.emplace(std::move(key), ""); // 使用 std::move 优化性能
            continue;
        }

        std::string &val = kv[1]; // 获取值

        // 如果值为空，将键添加到映射表中，值为空字符串
        if (val.empty())
            res.emplace(std::move(key), "");
        else
            // 将键值对添加到映射表中
            res.emplace(std::move(key), std::move(val));
    }

    // 返回解析后的键值对映射表
    return res;
}