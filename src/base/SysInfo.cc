#include <type_traits>  // 包含标准库中的type_traits头文件，用于类型特性检查
#include "SysInfo.h"  // 包含SysInfo头文件，声明了CurrentThread的相关功能

namespace Yukino  // 定义一个命名空间Yukino，用于封装相关的类和函数
{  // 命名空间的开始

namespace CurrentThread  // 定义一个命名空间CurrentThread，用于封装线程相关功能
{  // 命名空间的开始

// 定义线程局部存储变量，用于缓存当前线程的线程ID
thread_local int t_cached_tid = 0;  // 初始化为0，表示尚未缓存线程ID

// 定义线程局部存储变量，用于缓存当前线程的线程ID字符串
thread_local char t_tid_str[32];  // 字符数组，长度为32，足够存储线程ID字符串

// 定义线程局部存储变量，用于缓存当前线程的线程ID字符串长度
thread_local int t_tid_str_len = 6;  // 初始化为6，表示默认的线程ID字符串长度

// 静态断言，检查pid_t是否为int类型
// 如果pid_t不是int类型，编译器将报错
static_assert(std::is_same<int, pid_t>::value, "pid_t should be int");

}  // namespace CurrentThread  // 命名空间的结束
}  // namespace Yukino  // 命名空间的结束