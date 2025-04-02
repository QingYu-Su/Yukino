#ifndef YUKINO_STATUSCODE_H_  // 防止头文件重复包含的宏定义
#define YUKINO_STATUSCODE_H_

namespace Yukino  // 定义一个命名空间Yukino，用于封装相关的错误码和函数
{  // 命名空间的开始

// 定义一个枚举类型ErrorCode，用于表示不同的错误状态码
enum ErrorCode
{
    StatusOK = 0,  // 表示操作成功，无错误
    StatusNotFound,  // 表示未找到指定的资源或对象

    // 压缩相关的错误码
    StatusCompressError,  // 压缩过程中发生错误
    StatusCompressNotSupport,  // 不支持的压缩格式或方法
    StatusNoComrpess,  // 未启用压缩功能（拼写错误，应为StatusNoCompress）
    StatusUncompressError,  // 解压缩过程中发生错误
    StatusUncompressNotSupport,  // 不支持的解压缩格式或方法
    StatusNoUncomrpess,  // 未启用解压缩功能（拼写错误，应为StatusNoUncompress）

    // 文件操作相关的错误码
    StatusFileRangeInvalid,  // 文件范围无效（例如，读取超出文件大小的范围）
    StatusFileReadError,  // 文件读取失败
    StatusFileWriteError,  // 文件写入失败

    // JSON操作相关的错误码
    StatusJsonInvalid,  // JSON格式无效或解析失败

    // 代理相关的错误码
    StatusProxyError,  // 代理操作失败

    // 路由相关的错误码
    StatusRouteVerbNotImplment,  // 路由的HTTP方法未实现（拼写错误，应为StatusRouteVerbNotImplemented）
    StatusRouteNotFound,  // 未找到指定的路由
};

// 声明一个函数，用于将错误码转换为对应的字符串描述
// 参数：code - 错误码
// 返回值：指向错误码描述的字符串
const char* error_code_to_str(int code);

} // namespace Yukino  // 命名空间的结束

#endif // YUKINO_STATUSCODE_H_  // 宏定义的结束