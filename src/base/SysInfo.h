#ifndef YUKINO_SYSINFO_H_  // 防止头文件重复包含的宏定义
#define YUKINO_SYSINFO_H_

#include <ctime>  // 包含标准库中的ctime头文件，用于时间相关函数
#include <unistd.h>  // 包含POSIX标准定义的符号常量和类型
#include <sys/syscall.h>  // 包含系统调用的定义
#include <cstdio>  // 包含标准输入输出库

namespace Yukino  // 定义一个命名空间Yukino，用于封装相关的类和函数
{  // 命名空间的开始

namespace CurrentThread  // 定义一个命名空间CurrentThread，用于封装线程相关功能
{  // 命名空间的开始

// 内部变量，用于缓存当前线程的线程ID
extern thread_local int t_cached_tid;  // 线程局部存储，缓存的线程ID
extern thread_local char t_tid_str[32];  // 线程局部存储，缓存的线程ID字符串
extern thread_local int t_tid_str_len;  // 线程局部存储，缓存的线程ID字符串长度

// 获取当前线程的线程ID
inline pid_t gettid() { return static_cast<pid_t>(::syscall(SYS_gettid)); }

// 缓存当前线程的线程ID
inline void cacheTid()
{
    if (t_cached_tid == 0)  // 如果尚未缓存线程ID
    {
        t_cached_tid = gettid();  // 获取当前线程的线程ID
        t_tid_str_len = snprintf(t_tid_str, sizeof t_tid_str, "%5d ", t_cached_tid);  // 格式化线程ID为字符串
    }
}

// 获取当前线程的线程ID
inline int tid()
{
    if (__builtin_expect(t_cached_tid == 0, 0))  // 如果尚未缓存线程ID
    {
        cacheTid();  // 缓存线程ID
    }
    return t_cached_tid;  // 返回缓存的线程ID
}

// 获取当前线程的线程ID字符串（用于日志记录）
inline const char *tid_str() { return t_tid_str; }

// 获取当前线程的线程ID字符串长度（用于日志记录）
inline int tid_str_len() { return t_tid_str_len; }

}  // namespace CurrentThread  // 命名空间的结束

}  // namespace Yukino  // 命名空间的结束

#endif // YUKINO_SYSINFO_H_  // 宏定义的结束