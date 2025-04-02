#include <sys/stat.h>
#include <cstring>
#include "FileUtil.h"
#include "ErrorCode.h"
#include "PathUtil.h"

using namespace Yukino;

// 获取文件的大小
int FileUtil::size(const std::string &path, size_t *size)
{
    // 定义一个 stat 结构体，用于存储文件状态信息
    struct stat st;
    // 使用 memset 将结构体清零，确保所有字段初始化为 0
    memset(&st, 0, sizeof(st));

    // 调用 stat 函数获取文件状态信息
    int ret = stat(path.c_str(), &st);

    // 初始化状态码为 StatusOK（假设在 ErrorCode.h 中定义）
    int status = StatusOK;

    // 检查 stat 函数的返回值
    if (ret == -1)
    {
        // 如果 stat 返回 -1，表示文件不存在或无法访问
        *size = 0; // 将文件大小设置为 0
        status = StatusNotFound; // 设置状态码为 StatusNotFound
    }
    else
    {
        // 如果 stat 成功，获取文件大小
        *size = st.st_size; // st.st_size 是文件的大小
    }

    // 返回状态码
    return status;
}

// 检查文件是否存在
bool FileUtil::file_exists(const std::string &path)
{
    // 调用 PathUtil::is_file 函数检查路径是否为文件
    // 如果是文件，说明文件存在
    return PathUtil::is_file(path);
}