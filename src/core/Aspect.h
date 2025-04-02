#ifndef YUKINO_ASPECT_H_
#define YUKINO_ASPECT_H_

#include <vector>

namespace Yukino
{
    // 前向声明 HttpReq 和 HttpResp 类
    // 这些类可能在其他头文件中定义，此处仅声明以便在 Aspect 类中使用指针类型
    class HttpReq;
    class HttpResp;

    /**
     * @brief Aspect 类，定义了 HTTP 请求处理的前置和后置操作接口。
     *
     * Aspect 类是一个抽象基类，用于实现面向切面编程（AOP）的模式。
     * 它允许在 HTTP 请求处理的前后插入自定义逻辑，例如日志记录、权限验证、性能监控等。
     * 子类需要继承 Aspect 并实现 before 和 after 方法，以完成具体的切面逻辑。
     */
    class Aspect
    {
    public:
        virtual ~Aspect() = default;  // 虚析构函数，确保派生类析构时能正确调用基类析构函数

        /**
         * @brief 在 HTTP 请求处理之前执行的逻辑。
         *
         * @param req 指向 HttpReq 对象的指针，表示当前的 HTTP 请求。
         * @param resp 指向 HttpResp 对象的指针，用于修改响应内容。
         * @return bool 返回 true 表示继续处理请求；返回 false 表示终止请求处理，并直接返回响应。
         */
        virtual bool before(const HttpReq *req, HttpResp *resp) = 0;

        /**
         * @brief 在 HTTP 请求处理之后执行的逻辑。
         *
         * @param req 指向 HttpReq 对象的指针，表示当前的 HTTP 请求。
         * @param resp 指向 HttpResp 对象的指针，用于修改响应内容。
         * @return bool 返回 true 表示正常处理完成；返回 false 表示发生异常或需要特殊处理。
         */
        virtual bool after(const HttpReq *req, HttpResp *resp) = 0;
    };

    /**
     * @brief GlobalAspect 类，用于管理全局的 Aspect 列表。
     *
     * GlobalAspect 是一个单例类，用于存储和管理所有全局的 Aspect 对象。
     * 它提供了一个静态方法 get_instance 来获取唯一的实例，并通过 aspect_list 成员变量存储 Aspect 对象指针。
     */
    class GlobalAspect
    {
    public:
        /**
         * @brief 获取 GlobalAspect 的唯一实例。
         *
         * @return GlobalAspect* 返回 GlobalAspect 的实例指针。
         */
        static GlobalAspect *get_instance();

        // 禁止拷贝构造和赋值操作，确保单例的唯一性
        GlobalAspect(const GlobalAspect&) = delete;
        GlobalAspect& operator=(const GlobalAspect&) = delete;

    public:
        /**
         * @brief 存储 Aspect 对象指针的列表。
         *
         * aspect_list 用于存储所有全局的 Aspect 对象指针，以便在 HTTP 请求处理过程中依次调用它们的 before 和 after 方法。
         */
        std::vector<Aspect *> aspect_list;

    private:
        GlobalAspect() = default;  // 私有构造函数，确保只能通过 get_instance 创建实例

        ~GlobalAspect();  // 私有析构函数，用于释放资源
    };

} // namespace Yukino

#endif // YUKINO_ASPECT_H_