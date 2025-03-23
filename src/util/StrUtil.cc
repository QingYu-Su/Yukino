#include "StrUtil.h"

using namespace Yukino;

// 定义全局常量字符串，表示“未找到”的情况
const std::string Yukino::string_not_found = "";

// 定义默认的成对字符字符串
const std::string StrUtil::k_pairs_ = R"({}[]()<>""''``)";

// 去除字符串两端的成对字符
StringPiece StrUtil::trim_pairs(const StringPiece &str, const char *pairs)
{
    // 获取字符串的起始和结束指针
    const char *lhs = str.begin();
    const char *rhs = str.begin() + str.size() - 1;

    // 遍历成对字符字符串
    const char *p = pairs;
    bool is_pair = false; // 标记是否找到成对字符
    while (*p != '\0' && *(p + 1) != '\0') // 确保成对字符的两个字符都存在
    {
        // 检查字符串的首尾字符是否与成对字符匹配
        if (*lhs == *p && *rhs == *(p + 1))
        {
            is_pair = true; // 找到匹配的成对字符
            break;
        }
        p += 2; // 跳过一对字符
    }

    // 如果找到成对字符，返回去除首尾字符后的子字符串
    // 否则返回原始字符串
    return is_pair ? StringPiece(str.begin() + 1, str.size() - 2) : str;
}

// 去除字符串左侧的空白字符
StringPiece StrUtil::ltrim(const StringPiece &str)
{
    // 获取字符串的起始指针
    const char *lhs = str.begin();

    // 遍历字符串，跳过左侧的空白字符
    while (lhs != str.end() && std::isspace(*lhs)) lhs++;

    // 如果所有字符都是空白字符，返回空字符串
    if (lhs == str.end()) return {};

    // 创建一个新的 StringPiece，去除左侧空白字符
    StringPiece res(str);
    res.remove_prefix(lhs - str.begin()); // 去除左侧空白字符
    return res;
}

// 去除字符串右侧的空白字符
StringPiece StrUtil::rtrim(const StringPiece &str)
{
    // 如果字符串为空，直接返回
    if (str.empty()) return str;

    // 获取字符串的结束指针
    const char *rhs = str.end() - 1;

    // 遍历字符串，跳过右侧的空白字符
    while (rhs != str.begin() && std::isspace(*rhs)) rhs--;

    // 如果所有字符都是空白字符，返回空字符串
    if (rhs == str.begin() && std::isspace(*rhs)) return {};

    // 创建一个新的 StringPiece，去除右侧空白字符
    StringPiece res(str.begin(), rhs - str.begin() + 1);
    return res;
}

// 去除字符串两端的空白字符
StringPiece StrUtil::trim(const StringPiece &str)
{
    // 先去除右侧空白字符，再去除左侧空白字符
    return ltrim(rtrim(str));
}