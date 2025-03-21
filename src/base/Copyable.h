#ifndef YUKINO_COPYABLE_H_  // 防止头文件重复包含的宏定义
#define YUKINO_COPYABLE_H_

namespace Yukino  // 定义一个命名空间Yukino，用于封装相关的类和函数
{  // 命名空间的开始

// 定义一个Copyable类，用于标记一个类是可以被复制的
class Copyable
{
protected:
    Copyable() = default;  // 默认构造函数，允许默认构造
    ~Copyable() = default;  // 默认析构函数，允许默认析构
};

} // namespace Yukino  // 命名空间的结束

#endif  // YUKINO_COPYABLE_H_  // 宏定义的结束