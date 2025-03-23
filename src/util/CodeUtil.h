// Modified from cinatra

#ifndef YUKINO_CODEUTIL_H_
#define YUKINO_CODEUTIL_H_

#include <cstddef>
#include <string>

namespace Yukino
{
    // 声明 StringPiece 类（假设在其他地方定义）
    class StringPiece;

    /**
     * @class CodeUtil
     * @brief 提供编码和解码相关的工具函数
     *
     * 该类包含静态方法，用于处理 URL 编码和解码操作，以及判断字符串是否为 URL 编码。
     */
    class CodeUtil
    {
    public:
        /**
         * @brief 对字符串进行 URL 编码
         *
         * 将输入字符串中的特殊字符（如空格、&、% 等）转换为对应的百分号编码形式。
         * 例如，空格会被编码为 "%20"。
         *
         * @param value 要进行 URL 编码的字符串
         * @return 返回 URL 编码后的字符串
         */
        static std::string url_encode(const std::string &value);

        /**
         * @brief 对字符串进行 URL 解码
         *
         * 将输入字符串中的百分号编码形式转换回原始字符。
         * 例如，"%20" 会被解码为空格。
         *
         * @param value 要进行 URL 解码的字符串
         * @return 返回 URL 解码后的字符串
         */
        static std::string url_decode(const std::string &value);

        /**
         * @brief 判断字符串是否为 URL 编码
         *
         * 检查字符串中是否包含 URL 编码的特征（如 "%XX" 形式的字符）。
         *
         * @param str 要检查的字符串
         * @return 如果字符串是 URL 编码的，返回 true；否则返回 false
         */
        static bool is_url_encode(const std::string &str);
    };

} // namespace Yukino


#endif // YUKINO_CODEUTIL_H_