#include "ErrorCode.h"  // 包含ErrorCode头文件，声明了ErrorCode枚举类型
#include <map>  // 包含标准库中的map头文件，用于创建映射表

namespace Yukino  // 使用Yukino命名空间，与头文件中的命名空间一致
{  // 命名空间的开始

// 定义一个全局的错误码映射表，用于将错误码映射到对应的字符串描述
// 键：错误码（int类型）
// 值：错误码的字符串描述（const char*类型）
std::map<int, const char *> error_code_table = {
    { StatusOK, "OK" },  // 操作成功
    { StatusCompressError, "Compress Error" },  // 压缩过程中发生错误
    { StatusCompressNotSupport, "Compress Not Support" },  // 不支持的压缩格式或方法
    { StatusNoComrpess, "No Comrpession" },  // 未启用压缩功能
    { StatusUncompressError, "Uncompress Error" },  // 解压缩过程中发生错误
    { StatusUncompressNotSupport, "Uncompress Not Support" },  // 不支持的解压缩格式或方法
    { StatusNoUncomrpess, "No Uncomrpess" },  // 未启用解压缩功能
    { StatusNotFound, "404 Not Found" },  // 未找到指定的资源或对象
    { StatusFileRangeInvalid, "File Range Invalid" },  // 文件范围无效
    { StatusFileReadError, "File Read Error" },  // 文件读取失败
    { StatusFileWriteError, "File Write Error" },  // 文件写入失败
    { StatusJsonInvalid, "Invalid Json Syntax" },  // JSON格式无效或解析失败
    { StatusProxyError, "Http Proxy Error" },  // 代理操作失败
    { StatusRouteVerbNotImplment, "Route Http Method not implement" },  // 路由的HTTP方法未实现
    { StatusRouteNotFound, "Route Not Found" },  // 未找到指定的路由
};

// 实现将错误码转换为字符串描述的函数
// 参数：code - 要转换的错误码
// 返回值：错误码对应的字符串描述
const char* error_code_to_str(int code)
{
    auto it = error_code_table.find(code);  // 在映射表中查找指定的错误码
    if(it == error_code_table.end())  // 如果未找到对应的错误码
        return "unknown";  // 返回"unknown"作为默认值
    return it->second;  // 返回错误码对应的字符串描述
}

} // namespace Yukino  // 命名空间的结束