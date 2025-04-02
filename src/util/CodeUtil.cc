#include "CodeUtil.h"
#include "StringPiece.h"

namespace Yukino
{

// URL 编码函数实现
std::string CodeUtil::url_encode(const std::string &value)
{
    static auto hex_chars = "0123456789ABCDEF"; // 静态变量，存储十六进制字符

    std::string result;
    result.reserve(value.size()); // 预分配内存，避免频繁扩容

    // 遍历输入字符串的每个字符
    for (auto &chr : value) 
    {
        // 检查字符是否是 URL 安全字符（数字、字母、部分特殊字符）
        if (!((chr >= '0' && chr <= '9') || (chr >= 'A' && chr <= 'Z') ||
            (chr >= 'a' && chr <= 'z') || chr == '-' || chr == '.' ||
            chr == '_' || chr == '~' || chr == '/'))
        {
            // 如果不是安全字符，进行 URL 编码
            // 将字符转换为两位十六进制表示
            result += std::string("%") +
                    hex_chars[static_cast<unsigned char>(chr) >> 4] + // 高四位
                    hex_chars[static_cast<unsigned char>(chr) & 15]; // 低四位
        } else
        {
            // 如果是安全字符，直接添加到结果中
            result += chr;
        }   
    }

    return result; // 返回编码后的字符串
}

// URL 解码函数实现
std::string CodeUtil::url_decode(const std::string &value)
{
    std::string result;
    result.reserve(value.size() / 3 + (value.size() % 3)); // 预分配内存，避免频繁扩容

    // 遍历输入字符串的每个字符
    for (std::size_t i = 0; i < value.size(); ++i) 
    {
        auto &chr = value[i];
        // 如果遇到 '%'，表示接下来的两个字符是十六进制编码
        if (chr == '%' && i + 2 < value.size()) 
        {
            auto hex = value.substr(i + 1, 2); // 提取两位十六进制字符
            auto decoded_chr =
                static_cast<char>(std::strtol(hex.c_str(), nullptr, 16)); // 转换为字符
            result += decoded_chr; // 添加解码后的字符到结果中
            i += 2; // 跳过两位十六进制字符
        } else if (chr == '+')
        {
            // 如果遇到 '+'，解码为空格
            result += ' ';
        } else
        {
            // 其他字符直接添加到结果中
            result += chr;
        }
    }
    return result; // 返回解码后的字符串
}

// 判断字符串是否为 URL 编码的函数实现
bool CodeUtil::is_url_encode(const std::string &str)
{
    // 如果字符串中包含 '%' 或 '+'，则认为是 URL 编码的
    return str.find("%") != std::string::npos ||
           str.find("+") != std::string::npos;
}

}  // namespace Yukino