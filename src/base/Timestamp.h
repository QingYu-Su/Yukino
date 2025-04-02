#ifndef YUKINO_TIMESTAMP_H_
#define YUKINO_TIMESTAMP_H_

#include <chrono>  // 包含标准库中的chrono头文件，用于时间相关功能
#include <string>  // 包含标准库中的string头文件，用于字符串操作
#include <sstream> // 包含标准库中的sstream头文件，用于字符串流操作
#include <iomanip> // 包含标准库中的iomanip头文件，用于格式化输出

#include "Copyable.h"  // 包含Copyable类，表示Timestamp是可复制的

using namespace std::chrono;  // 使用std::chrono命名空间

namespace Yukino
{
class Timestamp : public Copyable
{
public:
    // 默认构造函数，表示无效的时间戳
    Timestamp();

    // 拷贝构造函数
    Timestamp(const Timestamp &that);

    // 拷贝赋值运算符
    Timestamp &operator=(const Timestamp &that);

    // 显式构造函数，从1970-01-01 00:00:00开始的微秒数
    explicit Timestamp(uint64_t micro_sec_since_epoch);

    // 交换两个Timestamp对象
    void swap(Timestamp &that);

    // 将时间戳转换为字符串形式
    std::string to_str() const;

    // 将时间戳转换为格式化的字符串形式，默认格式
    std::string to_format_str() const;

    // 将时间戳转换为指定格式的字符串形式
    std::string to_format_str(const char *fmt) const;

    // 获取从1970-01-01 00:00:00开始的微秒数
    uint64_t micro_sec_since_epoch() const;

    // 检查时间戳是否有效
    bool valid() const
    { return micro_sec_since_epoch_ > 0; }

    // 获取当前时间的时间戳
    static Timestamp now();

    // 获取一个无效的时间戳
    static Timestamp invalid()
    { return Timestamp(); }

    // 每秒的微秒数
    static const int k_micro_sec_per_sec = 1000 * 1000;
private:
    uint64_t micro_sec_since_epoch_;  // 从1970-01-01 00:00:00开始的微秒数
};

// 定义比较操作符
inline bool operator<(Timestamp lhs, Timestamp rhs)
{
    return lhs.micro_sec_since_epoch() < rhs.micro_sec_since_epoch();
}

inline bool operator>(Timestamp lhs, Timestamp rhs)
{
    return lhs.micro_sec_since_epoch() > rhs.micro_sec_since_epoch();
}

inline bool operator<=(Timestamp lhs, Timestamp rhs)
{
    return lhs.micro_sec_since_epoch() <= rhs.micro_sec_since_epoch();
}

inline bool operator>=(Timestamp lhs, Timestamp rhs)
{
    return lhs.micro_sec_since_epoch() >= rhs.micro_sec_since_epoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
    return lhs.micro_sec_since_epoch() == rhs.micro_sec_since_epoch();
}

inline bool operator!=(Timestamp lhs, Timestamp rhs)
{
    return lhs.micro_sec_since_epoch() != rhs.micro_sec_since_epoch();
}

// 定义加法操作符
inline Timestamp operator+(Timestamp lhs, uint64_t ms)
{
    return Timestamp(lhs.micro_sec_since_epoch() + ms);
}

inline Timestamp operator+(Timestamp lhs, double seconds)
{
    uint64_t delta = static_cast<uint64_t>(seconds * Timestamp::k_micro_sec_per_sec);
    return Timestamp(lhs.micro_sec_since_epoch() + delta);
}

// 定义减法操作符
inline Timestamp operator-(Timestamp lhs, uint64_t ms)
{
    return Timestamp(lhs.micro_sec_since_epoch() - ms);
}

inline Timestamp operator-(Timestamp lhs, double seconds)
{
    uint64_t delta = static_cast<uint64_t>(seconds * Timestamp::k_micro_sec_per_sec);
    return Timestamp(lhs.micro_sec_since_epoch() - delta);
}

// 计算两个时间戳之间的差值（以秒为单位）
inline double operator-(Timestamp high, Timestamp low)
{
    uint64_t diff = high.micro_sec_since_epoch() - low.micro_sec_since_epoch();
    return static_cast<double>(diff) / Timestamp::k_micro_sec_per_sec;
}

}  // namespace Yukino

#endif // YUKINO_TIMESTAMP_H_