#include "base64.h"  // 包含Base64头文件，声明了Base64类及其接口
using namespace Yukino;  // 使用Yukino命名空间，避免重复声明

namespace  // 匿名命名空间，用于定义局部函数和变量
{
    // 判断字符是否是Base64编码中合法的字符
    // 参数：c - 要判断的字符
    // 返回值：如果字符是Base64合法字符，则返回true，否则返回false
    bool is_base64(unsigned char c) {
        return (isalnum(c) || (c == '+') || (c == '/'));  // 检查字符是否是字母、数字、'+'或'/'
    }
} // namespace

// Base64类中静态成员变量的定义，存储Base64编码所使用的字符集
const std::string Base64::base64_chars =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"  // 大写字母
                "abcdefghijklmnopqrstuvwxyz"  // 小写字母
                "0123456789+/" ;              // 数字和特殊字符 '+' 和 '/'

// Base64编码函数的实现
std::string Base64::encode(const unsigned char *bytes_to_encode, unsigned int len)
{
    std::string ret;  // 存储编码后的结果字符串
    int i = 0;        // 用于处理每3个字节的索引
    int j = 0;        // 用于处理剩余字节的索引
    unsigned char char_array_3[3];  // 用于存储每3个字节的临时数组
    unsigned char char_array_4[4];  // 用于存储编码后的4个Base64字符的临时数组

    // 遍历输入的字节数组，每次处理3个字节
    while (len--) {
        char_array_3[i++] = *(bytes_to_encode++);  // 将当前字节存入临时数组
        if (i == 3) {  // 如果已经处理了3个字节
            // 将3个字节转换为4个Base64字符
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;  // 第1个字节的高6位
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);  // 第1个字节的低2位和第2个字节的高4位
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);  // 第2个字节的低4位和第3个字节的高2位
            char_array_4[3] = char_array_3[2] & 0x3f;  // 第3个字节的低6位

            // 将4个Base64字符添加到结果字符串中
            for(i = 0; (i < 4) ; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;  // 重置索引，准备处理下一批3个字节
        }
    }

    // 如果最后不足3个字节，进行特殊处理
    if (i) {
        for(j = i; j < 3; j++)  // 将剩余字节不足的部分补0
            char_array_3[j] = '\0';

        // 按照Base64编码规则处理剩余的字节
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        // 将处理后的Base64字符添加到结果字符串中
        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        // 添加'='作为填充字符，直到达到4的倍数
        while((i++ < 3))
            ret += '=';
    }

    return ret;  // 返回编码后的Base64字符串
}

// Base64解码函数的实现
std::string Base64::decode(const std::string &encoded_string)
{
    int in_len = encoded_string.size();  // 获取输入字符串的长度
    int i = 0;  // 用于处理每4个Base64字符的索引
    int j = 0;  // 用于处理剩余字符的索引
    int in_ = 0;  // 输入字符串的索引
    unsigned char char_array_4[4];  // 用于存储每4个Base64字符的临时数组
    unsigned char char_array_3[3];  // 用于存储解码后的3个字节的临时数组
    std::string ret;  // 存储解码后的结果字符串

    // 遍历输入的Base64字符串，每次处理4个字符
    while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_];  // 将当前字符存入临时数组
        in_++;  // 移动到下一个字符
        if (i == 4) {  // 如果已经处理了4个Base64字符
            // 将Base64字符转换为对应的索引值
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            // 将4个Base64字符解码为3个字节
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            // 将解码后的字节添加到结果字符串中
            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;  // 重置索引，准备处理下一批4个Base64字符
        }
    }

    // 如果最后不足4个Base64字符，进行特殊处理
    if (i) {
        for (j = i; j < 4; j++)  // 将剩余字符不足的部分补0
            char_array_4[j] = 0;

        // 将Base64字符转换为对应的索引值
        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        // 按照Base64解码规则处理剩余的字符
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        // 将解码后的字节添加到结果字符串中
        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;  // 返回解码后的字符串
}