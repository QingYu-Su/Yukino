#ifndef YUKINO_STRUTIL_H_
#define YUKINO_STRUTIL_H_

#include "workflow/StringUtil.h"
#include <string>
#include "StringPiece.h"

namespace Yukino
{
    // 定义一个常量字符串，用于表示“未找到”的情况
    extern const std::string string_not_found;

    /**
     * @class StrUtil
     * @brief 提供字符串处理工具函数
     *
     * 该类继承自 StringUtil，并扩展了一些字符串处理功能，包括去除空白字符、分割字符串等。
     */
    class StrUtil : public StringUtil
    {
    public:
        /**
         * @brief 去除字符串两端的成对字符
         *
         * 从字符串两端去除指定的成对字符（如引号、括号等）。
         *
         * @param str 输入字符串
         * @param pairs 成对字符的字符串，默认值为 k_pairs_
         * @return 返回去除成对字符后的字符串片段
         */
        static StringPiece trim_pairs(const StringPiece &str, const char *pairs = k_pairs_.c_str());

        /**
         * @brief 去除字符串左侧的空白字符
         *
         * 从字符串左侧去除空白字符（空格、制表符等）。
         *
         * @param str 输入字符串
         * @return 返回去除左侧空白后的字符串片段
         */
        static StringPiece ltrim(const StringPiece &str);

        /**
         * @brief 去除字符串右侧的空白字符
         *
         * 从字符串右侧去除空白字符（空格、制表符等）。
         *
         * @param str 输入字符串
         * @return 返回去除右侧空白后的字符串片段
         */
        static StringPiece rtrim(const StringPiece &str);

        /**
         * @brief 去除字符串两端的空白字符
         *
         * 从字符串两端去除空白字符（空格、制表符等）。
         *
         * @param str 输入字符串
         * @return 返回去除两端空白后的字符串片段
         */
        static StringPiece trim(const StringPiece &str);

        /**
         * @brief 按分隔符分割字符串
         *
         * 按指定分隔符将字符串分割为多个子字符串，并存储到向量中。
         *
         * @param str 输入字符串
         * @param sep 分隔符
         * @return 返回分割后的字符串向量
         */
        template<class OutputStringType>
        static std::vector<OutputStringType> split_piece(const StringPiece &str, char sep);

    private:
        /**
         * @brief 默认的成对字符字符串
         *
         * 用于 trim_pairs 函数的默认成对字符，例如引号、括号等。
         */
        static const std::string k_pairs_;
    };

    /**
     * @brief 按分隔符分割字符串的实现
     *
     * 按指定分隔符将字符串分割为多个子字符串，并存储到向量中。
     *
     * @param str 输入字符串
     * @param sep 分隔符
     * @return 返回分割后的字符串向量
     */
    template<class OutputStringType>
    std::vector<OutputStringType> StrUtil::split_piece(const StringPiece &str, char sep)
    {
        std::vector<OutputStringType> res;
        if (str.empty())
            return res;

        const char *p = str.begin();
        const char *cursor = p;

        while (p != str.end())
        {
            if (*p == sep)
            {
                res.emplace_back(OutputStringType(cursor, p - cursor)); // 添加子字符串到结果向量
                cursor = p + 1; // 更新游标位置
            }
            ++p;
        }
        res.emplace_back(OutputStringType(cursor, str.end() - cursor)); // 添加最后一部分
        return res;
    }

    /**
     * @class MapStringCaseLess
     * @brief 提供不区分大小写的字符串比较函数
     *
     * 用于 std::map 等容器，使得字符串键不区分大小写。
     */
    class MapStringCaseLess {
    public:
        /**
         * @brief 比较两个字符串
         *
         * 使用 strcasecmp 函数比较两个字符串，不区分大小写。
         *
         * @param lhs 左侧字符串
         * @param rhs 右侧字符串
         * @return 如果 lhs < rhs，返回 true；否则返回 false
         */
        bool operator()(const std::string& lhs, const std::string& rhs) const {
            return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
        }
    };

}  // namespace Yukino

#endif // YUKINO_STRUTIL_H_