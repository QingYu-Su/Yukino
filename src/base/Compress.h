// Modified from drogon
// https://zlib.net/manual.html

#ifndef YUKINO_COMPRESS_H_  // 防止头文件重复包含的宏定义
#define YUKINO_COMPRESS_H_

#include <string>  // 包含标准库中的string头文件，用于处理字符串
#include <zlib.h>  // 包含zlib库的头文件，用于压缩和解压缩功能

namespace Yukino  // 定义一个命名空间Yukino，用于封装相关的类和函数
{  // 命名空间的开始

// 定义一个枚举类，用于表示支持的压缩方法
enum class Compress 
{
    GZIP  // 表示GZIP压缩方法
};

// 声明一个函数，用于将压缩方法枚举值转换为对应的字符串描述
const char* compress_method_to_str(const Compress& compress_method);

// 定义一个Compressor类，封装压缩和解压缩功能
class Compressor
{
public:
    // 静态成员函数，使用GZIP方法对字符串进行压缩
    // 参数：
    // - src：指向要压缩的字符串的指针
    // - dest：指向存储压缩结果的字符串的指针
    // 返回值：压缩操作的结果代码（成功或失败）
    static int gzip(const std::string * const src, std::string *dest);

    // 静态成员函数，使用GZIP方法对字节数组进行压缩
    // 参数：
    // - data：指向要压缩的字节数组的指针
    // - len：字节数组的长度
    // - dest：指向存储压缩结果的字符串的指针
    // 返回值：压缩操作的结果代码（成功或失败）
    static int gzip(const char *data, const size_t len, std::string *dest);

    // 静态成员函数，使用GZIP方法对字符串进行解压缩
    // 参数：
    // - src：指向要解压缩的字符串的指针
    // - dest：指向存储解压缩结果的字符串的指针
    // 返回值：解压缩操作的结果代码（成功或失败）
    static int ungzip(const std::string * const src, std::string *dest);
    
    // 静态成员函数，使用GZIP方法对字节数组进行解压缩
    // 参数：
    // - data：指向要解压缩的字节数组的指针
    // - len：字节数组的长度
    // - dest：指向存储解压缩结果的字符串的指针
    // 返回值：解压缩操作的结果代码（成功或失败）
    static int ungzip(const char *data, const size_t len, std::string *dest);
};

}  // namespace Yukino  // 命名空间的结束

#endif // YUKINO_COMPRESS_H_  // 宏定义的结束