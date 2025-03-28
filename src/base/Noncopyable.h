#ifndef YUKINO_NONCOPYABLE_H_  // 防止头文件重复包含的宏定义
#define YUKINO_NONCOPYABLE_H_

namespace Yukino  // 定义一个命名空间Yukino，用于封装相关的类和函数
{  // 命名空间的开始

// 定义一个Noncopyable类，用于禁止类的复制构造和赋值操作
class Noncopyable
{
public:
    // 禁止复制构造函数
    Noncopyable(const Noncopyable&) = delete;

    // 禁止赋值运算符
    void operator=(const Noncopyable&) = delete;

protected:
    // 默认构造函数，允许默认构造
    Noncopyable() = default;

    // 默认析构函数，允许默认析构
    ~Noncopyable() = default;
};

} // namespace Yukino  // 命名空间的结束

#endif  // YUKINO_NONCOPYABLE_H_  // 宏定义的结束