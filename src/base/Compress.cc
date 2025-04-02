
#include <cassert>  // 包含断言头文件，用于调试时检查条件
#include "Compress.h"  // 包含Compress类的声明
#include "ErrorCode.h"  // 包含错误码的定义

namespace Yukino  // 使用Yukino命名空间
{
    // 将压缩方法枚举值转换为字符串描述的函数实现
    const char* compress_method_to_str(const Compress& compress_method)
    {
        switch (compress_method)
        {
            case Compress::GZIP:  // 如果是GZIP压缩方法
                return "gzip";  // 返回字符串描述
            default:  // 其他未定义的压缩方法
                return "unsupport compression";  // 返回不支持的提示
        }
    }
}  // namespace Yukino

using namespace Yukino;  // 使用Yukino命名空间，避免重复声明

// GZIP压缩函数的实现
int Compressor::gzip(const std::string * const src, std::string *dest)
{
    const char *data = src->c_str();  // 获取输入字符串的C风格指针
    const size_t len = src->size();  // 获取输入字符串的长度
    return gzip(data, len, dest);  // 调用另一个重载的gzip函数
}

// GZIP压缩函数的实现（重载版本）
int Compressor::gzip(const char *data, const size_t len, std::string *dest)
{
    dest->clear();  // 清空目标字符串
    z_stream strm = {nullptr,  // 初始化z_stream结构体
                     0,
                     0,
                     nullptr,
                     0,
                     0,
                     nullptr,
                     nullptr,
                     nullptr,
                     nullptr,
                     nullptr,
                     0,
                     0,
                     0};
    if (data && len > 0)  // 检查输入数据是否有效
    {
        if (deflateInit2(&strm,  // 初始化压缩流
                         Z_DEFAULT_COMPRESSION,  // 默认压缩级别
                         Z_DEFLATED,  // 使用DEFLATE算法
                         MAX_WBITS + 16,  // 窗口大小（支持GZIP格式）
                         8,  // 内存级别
                         Z_DEFAULT_STRATEGY) != Z_OK)  // 默认压缩策略
        {
            // fprintf(stderr, "deflateInit2 error!\n");
            return StatusCompressError;  // 初始化失败，返回错误码
        }
        std::string outstr;  // 创建输出字符串
        outstr.resize(compressBound(static_cast<uLong>(len)));  // 根据输入长度预分配空间
        strm.next_in = (Bytef *)data;  // 设置输入数据指针
        strm.avail_in = static_cast<uInt>(len);  // 设置输入数据长度
        int ret;
        do
        {
            if (strm.total_out >= outstr.size())  // 如果输出空间不足
            {
                outstr.resize(strm.total_out * 2);  // 扩大输出空间
            }
            assert(outstr.size() >= strm.total_out);  // 断言输出空间足够
            strm.avail_out = static_cast<uInt>(outstr.size() - strm.total_out);  // 更新剩余输出空间
            strm.next_out = (Bytef *)outstr.data() + strm.total_out;  // 更新输出数据指针
            ret = deflate(&strm, Z_FINISH); /* no bad return value */  // 执行压缩操作
            if (ret == Z_STREAM_ERROR)  // 如果发生错误
            {
                (void)deflateEnd(&strm);  // 清理压缩流
                return StatusCompressError;  // 返回错误码
            }
        } while (strm.avail_out == 0);  // 如果输出空间用完，继续循环
        assert(strm.avail_in == 0);  // 断言输入数据已处理完
        assert(ret == Z_STREAM_END); /* stream will be complete */  // 断言压缩完成
        outstr.resize(strm.total_out);  // 调整输出字符串大小
        /* clean up and return */
        (void)deflateEnd(&strm);  // 清理压缩流
        *dest = std::move(outstr);  // 将压缩结果移动到目标字符串
        return StatusOK;  // 返回成功码
    }
    return StatusCompressError;  // 输入数据无效，返回错误码
}

// GZIP解压缩函数的实现
int Compressor::ungzip(const std::string * const src, std::string *dest)
{
    const char *data = src->c_str();  // 获取输入字符串的C风格指针
    const size_t len = src->size();  // 获取输入字符串的长度
    return ungzip(data, len, dest);  // 调用另一个重载的ungzip函数
}

// GZIP解压函数的实现（重载版本）
int Compressor::ungzip(const char *data, const size_t len, std::string *dest)
{
    // 清空目标字符串，确保不会包含旧数据
    dest->clear();

    // 如果输入数据长度为0，直接返回成功，因为没有数据需要解压缩
    if (len == 0)
        return StatusOK;

    // 保存输入数据的长度
    auto full_length = len;

    // 预分配解压缩后的数据空间，初始大小为输入数据长度的两倍
    // 这是为了确保有足够的空间来存储解压缩后的数据
    auto decompressed = std::string(full_length * 2, 0);

    // 标记解压缩是否完成
    bool done = false;

    // 初始化z_stream结构体，用于解压缩操作
    z_stream strm = {nullptr,
                     0,
                     0,
                     nullptr,
                     0,
                     0,
                     nullptr,
                     nullptr,
                     nullptr,
                     nullptr,
                     nullptr,
                     0,
                     0,
                     0};
    strm.next_in = (Bytef *)data;  // 设置输入数据指针
    strm.avail_in = static_cast<uInt>(len);  // 设置输入数据长度
    strm.total_out = 0;  // 初始化输出数据长度
    strm.zalloc = Z_NULL;  // 设置内存分配函数为默认
    strm.zfree = Z_NULL;  // 设置内存释放函数为默认

    // 初始化解压缩流，支持GZIP格式（15 + 32）
    // 15是窗口大小的对数，32表示支持GZIP格式
    if (inflateInit2(&strm, (15 + 32)) != Z_OK)
    {
        // fprintf(stderr, "inflateInit2 error!\n");
        // 初始化失败，返回解压缩错误码
        return StatusUncompressError;
    }

    // 循环直到解压缩完成
    while (!done)
    {
        // 检查输出缓冲区是否足够，如果不足则扩大缓冲区
        if (strm.total_out >= decompressed.length())
        {
            // 扩大缓冲区大小为当前大小的两倍
            decompressed.resize(decompressed.length() * 2);
        }
        // 更新输出数据指针
        strm.next_out = (Bytef *)decompressed.data() + strm.total_out;
        // 更新剩余输出空间
        strm.avail_out = static_cast<uInt>(decompressed.length() - strm.total_out);

        // 执行解压缩操作
        int status = inflate(&strm, Z_SYNC_FLUSH);
        if (status == Z_STREAM_END)  // 如果解压缩完成
        {
            done = true;  // 设置完成标志
        }
        else if (status != Z_OK)  // 如果发生错误
        {
            break;  // 退出循环
        }
    }

    // 清理解压缩流
    if (inflateEnd(&strm) != Z_OK)
        // 清理失败，返回解压缩错误码
        return StatusUncompressError;

    // 设置解压缩后的数据长度
    int status = StatusOK;  // 初始化状态为成功
    if (done)  // 如果解压缩完成
    {
        // 调整解压缩数据的大小为实际输出长度
        decompressed.resize(strm.total_out);
        // 将解压缩数据移动到目标字符串
        *dest = std::move(decompressed);
    }
    else  // 如果解压缩未完成
    {
        // 设置状态为失败
        status = StatusUncompressError;
    }

    // 返回最终状态
    return status;
}

