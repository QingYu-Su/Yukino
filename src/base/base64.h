// modified from baidu lib
#ifndef YUKINO_BASE64_H_  // 防止头文件重复包含的宏定义
#define YUKINO_BASE64_H_

#include <string>  // 包含标准库中的string头文件，用于处理字符串操作

namespace Yukino  // 定义一个命名空间Yukino，用于封装相关的类和函数
{  // 命名空间的开始

class Base64  // 定义一个Base64类，用于实现Base64编码和解码功能
{
public:
    // 静态成员函数，用于将输入的字节数组进行Base64编码
    // 参数：
    // - bytes_to_encode：指向要编码的字节数组的指针
    // - len：字节数组的长度
    // 返回值：编码后的Base64字符串
    static std::string encode(const unsigned char *bytes_to_encode, unsigned int len);

    // 静态成员函数，用于将Base64编码的字符串进行解码
    // 参数：
    // - encoded_string：Base64编码的字符串
    // 返回值：解码后的原始字符串
    static std::string decode(std::string const &encoded_string);

private:
    // 静态常量成员变量，存储Base64编码所使用的字符集
    // 这些字符是Base64编码的基础，用于将二进制数据映射为可打印的字符
    static const std::string base64_chars;
};

}   // namespace Yukino  // 命名空间的结束

#endif // YUKINO_BASE64_H_  // 宏定义的结束