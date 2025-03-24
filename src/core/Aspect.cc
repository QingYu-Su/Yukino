#include "Aspect.h"

using namespace Yukino;

/**
 * @brief 实现 GlobalAspect 类的单例模式，返回唯一的实例。
 *
 * @return GlobalAspect* 返回 GlobalAspect 的唯一实例指针。
 *
 * @note 使用了 C++11 的静态局部变量特性，确保线程安全。
 *       静态局部变量在第一次调用时初始化，并且在程序结束时自动销毁。
 */
GlobalAspect *GlobalAspect::get_instance()
{
    static GlobalAspect kInstance;  // 静态局部变量，线程安全
    return &kInstance;              // 返回实例的地址
}

/**
 * @brief GlobalAspect 的析构函数，负责释放 aspect_list 中存储的 Aspect 对象。
 *
 * @note 遍历 aspect_list，调用 delete 释放每个 Aspect 对象的内存。
 *       这里假设 aspect_list 中存储的是动态分配的 Aspect 对象指针。
 */
GlobalAspect::~GlobalAspect()
{
    for (auto asp : aspect_list)  // 遍历 aspect_list
    {
        delete asp;  // 释放 Aspect 对象的内存
    }
}