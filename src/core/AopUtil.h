#ifndef YUKINO_AOPUTIL_H_
#define YUKINO_AOPUTIL_H_

#include <cstddef>
#include <tuple>
#include "Aspect.h"

namespace Yukino
{
    // 前向声明 HttpReq 和 HttpResp 类
    class HttpReq;
    class HttpResp;

    /**
     * @brief 递归模板函数，用于正向遍历 std::tuple 中的元素，并对每个元素执行指定的函数。
     *
     * @tparam I 当前遍历的索引，默认从 0 开始。
     * @tparam FuncT 要执行的函数类型。
     * @tparam Tp... 元组中的类型列表。
     * @param tp 要遍历的元组。
     * @param func 要对元组中的每个元素执行的函数。
     *
     * @note 使用了模板递归和 std::enable_if 来实现编译时的条件判断。
     *       当 I 等于元组大小时，停止递归。
     */
    template<std::size_t I = 0, typename FuncT, typename... Tp>
    inline typename std::enable_if<I == sizeof...(Tp), void>::type
    for_each(std::tuple<Tp...> &, FuncT)
    {}

    /**
     * @brief 递归模板函数，用于正向遍历 std::tuple 中的元素，并对每个元素执行指定的函数。
     *
     * @tparam I 当前遍历的索引，默认从 0 开始。
     * @tparam FuncT 要执行的函数类型。
     * @tparam Tp... 元组中的类型列表。
     * @param tp 要遍历的元组。
     * @param func 要对元组中的每个元素执行的函数。
     *
     * @note 使用了模板递归和 std::enable_if 来实现编译时的条件判断。
     *       当 I 小于元组大小时，继续递归。
     */
    template<std::size_t I = 0, typename FuncT, typename... Tp>
    inline typename std::enable_if<I < sizeof...(Tp), void>::type
    for_each(std::tuple<Tp...> &tp, FuncT func)
    {
        func(std::get<I>(tp));  // 对当前索引的元素执行函数
        for_each<I + 1, FuncT, Tp...>(tp, func);  // 递归调用，处理下一个元素
    }

    /**
     * @brief 递归模板函数，用于反向遍历 std::tuple 中的元素，并对每个元素执行指定的函数。
     *
     * @tparam I 当前遍历的索引，默认从 0 开始。
     * @tparam FuncT 要执行的函数类型。
     * @tparam Tp... 元组中的类型列表。
     * @param tp 要遍历的元组。
     * @param func 要对元组中的每个元素执行的函数。
     *
     * @note 使用了模板递归和 std::enable_if 来实现编译时的条件判断。
     *       当 I 等于元组大小时，停止递归。
     */
    template<std::size_t I = 0, typename FuncT, typename... Tp>
    inline typename std::enable_if<I == sizeof...(Tp), void>::type
    for_each_reverse(std::tuple<Tp...> &, FuncT)
    {}

    /**
     * @brief 递归模板函数，用于反向遍历 std::tuple 中的元素，并对每个元素执行指定的函数。
     *
     * @tparam I 当前遍历的索引，默认从 0 开始。
     * @tparam FuncT 要执行的函数类型。
     * @tparam Tp... 元组中的类型列表。
     * @param tp 要遍历的元组。
     * @param func 要对元组中的每个元素执行的函数。
     *
     * @note 使用了模板递归和 std::enable_if 来实现编译时的条件判断。
     *       当 I 小于元组大小时，继续递归。
     */
    template<std::size_t I = 0, typename FuncT, typename... Tp>
    inline typename std::enable_if<I < sizeof...(Tp), void>::type
    for_each_reverse(std::tuple<Tp...> &tp, FuncT func)
    {
        func(std::get<sizeof...(Tp) - 1 - I>(tp));  // 对当前索引的元素执行函数（从后往前）
        for_each_reverse<I + 1, FuncT, Tp...>(tp, func);  // 递归调用，处理下一个元素
    }

    /**
     * @brief 在 HTTP 请求处理之前，对元组中的每个 Aspect 对象调用 before 方法。
     *
     * @tparam Tuple 元组类型，包含多个 Aspect 对象。
     * @param req 指向 HttpReq 对象的指针，表示当前的 HTTP 请求。
     * @param resp 指向 HttpResp 对象的指针，用于修改响应内容。
     * @param tp 包含 Aspect 对象的元组。
     * @return bool 返回 true 表示所有 before 方法都成功执行；返回 false 表示有某个 before 方法失败。
     *
     * @note 使用了 for_each 函数正向遍历元组中的 Aspect 对象，并调用它们的 before 方法。
     */
    template<typename Tuple>
    inline bool aop_before(const HttpReq *req, HttpResp *resp, Tuple &tp)
    {
        bool ret = true;  // 初始化返回值为 true
        for_each(
                tp,
                [&ret, req, resp](Aspect &item)  // 捕获 ret、req 和 resp
                {
                    if (!ret)
                        return;  // 如果 ret 为 false，直接返回
                    ret = item.before(req, resp);  // 调用 Aspect 的 before 方法
                });
        return ret;  // 返回最终结果
    }

    /**
     * @brief 在 HTTP 请求处理之后，对元组中的每个 Aspect 对象调用 after 方法。
     *
     * @tparam Tuple 元组类型，包含多个 Aspect 对象。
     * @param req 指向 HttpReq 对象的指针，表示当前的 HTTP 请求。
     * @param resp 指向 HttpResp 对象的指针，用于修改响应内容。
     * @param tp 包含 Aspect 对象的元组。
     * @return bool 返回 true 表示所有 after 方法都成功执行；返回 false 表示有某个 after 方法失败。
     *
     * @note 使用了 for_each_reverse 函数反向遍历元组中的 Aspect 对象，并调用它们的 after 方法。
     */
    template<typename Tuple>
    inline bool aop_after(const HttpReq *req, HttpResp *resp, Tuple &tp)
    {
        bool ret = true;  // 初始化返回值为 true
        for_each_reverse(
                tp,
                [&ret, req, resp](Aspect &item)  // 捕获 ret、req 和 resp
                {
                    if (!ret)
                        return;  // 如果 ret 为 false，直接返回
                    ret = item.after(req, resp);  // 调用 Aspect 的 after 方法
                });
        return ret;  // 返回最终结果
    }

} // namespace Yukino

#endif // YUKINO_AOPUTIL_H_