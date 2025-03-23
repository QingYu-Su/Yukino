#include "PathUtil.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace Yukino;

// 获取路径中的文件名部分
std::string PathUtil::base(const std::string &filepath)
{
    // 找到最后一个非 '/' 字符的位置
    std::string::size_type pos1 = filepath.find_last_not_of("/");
    if (pos1 == std::string::npos)
    {
        // 如果路径全是 '/'，返回根目录 "/"
        return "/";
    }
    // 找到最后一个 '/' 的位置
    std::string::size_type pos2 = filepath.find_last_of("/", pos1);
    if (pos2 == std::string::npos)
    {
        // 如果没有找到 '/'，说明路径是文件名
        pos2 = 0;
    } else
    {
        // 跳过最后一个 '/'
        pos2++;
    }

    // 返回从最后一个 '/' 之后到结尾的子字符串
    return filepath.substr(pos2, pos1 - pos2 + 1);
}

// 获取路径中的文件扩展名
std::string PathUtil::suffix(const std::string& filepath)
{
    // 找到最后一个 '/' 的位置
    std::string::size_type pos1 = filepath.find_last_of("/");
    if (pos1 == std::string::npos) {
        // 如果没有 '/'，从头开始
        pos1 = 0;
    } else {
        // 跳过最后一个 '/'
        pos1++;
    }
    // 提取文件名部分
    std::string file = filepath.substr(pos1, -1);

    // 找到文件名中最后一个 '.' 的位置
    std::string::size_type pos2 = file.find_last_of(".");
    if (pos2 == std::string::npos) {
        // 如果没有找到 '.'，说明没有扩展名
        return "";
    }
    // 返回从 '.' 之后到结尾的子字符串作为扩展名
    return file.substr(pos2 + 1, -1);
}

// 拼接两个路径
std::string PathUtil::concat_path(const std::string &lhs, const std::string &rhs)
{
    std::string res;
    // 处理路径分隔符的情况
    if (lhs.back() == '/' && rhs.front() == '/') {
        // 如果左右路径都有 '/'，去掉一个
        res = lhs.substr(0, lhs.size() - 1) + rhs;
    } else if (lhs.back() != '/' && rhs.front() != '/') {
        // 如果左右路径都没有 '/'，添加一个
        res = lhs + "/" + rhs;
    } else {
        // 其他情况直接拼接
        res = lhs + rhs;
    }
    return res;
}

// 判断路径是否为目录
bool PathUtil::is_dir(const std::string &path)
{
    struct stat st;
    // 使用 stat 获取路径的状态信息
    return stat(path.c_str(), &st) >= 0 && S_ISDIR(st.st_mode);
}

// 判断路径是否为文件
bool PathUtil::is_file(const std::string &path)
{
    struct stat st;
    // 使用 stat 获取路径的状态信息
    return stat(path.c_str(), &st) >= 0 && S_ISREG(st.st_mode);
}