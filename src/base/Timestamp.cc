#include "Timestamp.h"

using namespace Yukino;

// 静态断言，确保Timestamp类的大小与uint64_t相同
static_assert(sizeof(Timestamp) == sizeof(uint64_t),
              "Timestamp should be same size as uint64_t");

// 默认构造函数，表示无效的时间戳
Timestamp::Timestamp()
        : micro_sec_since_epoch_(0)
{
}

// 从微秒数构造时间戳
Timestamp::Timestamp(uint64_t micro_sec_since_epoch)
        : micro_sec_since_epoch_(micro_sec_since_epoch)
{
}

// 拷贝构造函数
Timestamp::Timestamp(const Timestamp &that)
        : micro_sec_since_epoch_(that.micro_sec_since_epoch_)
{
}

// 拷贝赋值运算符
Timestamp &Timestamp::operator=(const Timestamp &that)
{
    micro_sec_since_epoch_ = that.micro_sec_since_epoch_;  // 拷贝微秒数
    return *this;  // 返回当前对象的引用
}

// 交换两个Timestamp对象
void Timestamp::swap(Timestamp &that)
{
    std::swap(micro_sec_since_epoch_, that.micro_sec_since_epoch_);  // 交换微秒数
}

// 将时间戳转换为字符串形式
std::string Timestamp::to_str() const
{
    // 将微秒数转换为秒和微秒部分，然后拼接成字符串
    return std::to_string(micro_sec_since_epoch_ / k_micro_sec_per_sec)  // 秒部分
           + "." + std::to_string(micro_sec_since_epoch_ % k_micro_sec_per_sec);  // 微秒部分
}

// 将时间戳转换为格式化的字符串形式，默认格式为"%Y-%m-%d %X"
std::string Timestamp::to_format_str() const
{
    return to_format_str("%Y-%m-%d %X");  // 调用带格式的函数
}

// 将时间戳转换为指定格式的字符串形式
std::string Timestamp::to_format_str(const char *fmt) const
{
    std::time_t time = micro_sec_since_epoch_ / k_micro_sec_per_sec;  // 将微秒转换为秒
    std::stringstream ss;  // 创建字符串流
    ss << std::put_time(std::localtime(&time), fmt);  // 使用put_time格式化时间
    return ss.str();  // 返回格式化后的字符串
}

// 获取从1970-01-01 00:00:00开始的微秒数
uint64_t Timestamp::micro_sec_since_epoch() const
{
    return micro_sec_since_epoch_;  // 返回微秒数
}

// 获取当前时间的时间戳
Timestamp Timestamp::now()
{
    // 使用std::chrono获取当前时间的微秒数
    uint64_t timestamp = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
    return Timestamp(timestamp);  // 返回时间戳对象
}