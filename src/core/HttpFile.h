#ifndef YUKINO_HTTPFILE_H_
#define YUKINO_HTTPFILE_H_

#include <string>
#include <vector>
#include <functional>

namespace Yukino
{
    class HttpResp; // 前向声明 HttpResp 类，表示 HTTP 响应对象

    class HttpFile
    {
    public:
        // 定义一个函数指针类型，用于文件 I/O 操作的回调函数
        using FileIOArgsFunc = std::function<void(const struct FileIOArgs*)>;

    public:
        // 发送文件内容到 HTTP 响应中
        // 参数：
        // - path: 文件路径
        // - start: 发送文件内容的起始位置
        // - end: 发送文件内容的结束位置
        // - resp: HTTP 响应对象
        // 返回值：操作结果，成功返回 0，失败返回其他值
        static int send_file(const std::string &path, size_t start, size_t end, HttpResp *resp);

        // 保存内容到文件中，并将结果反馈到 HTTP 响应对象
        // 参数：
        // - dst_path: 目标文件路径
        // - content: 要保存的内容
        // - resp: HTTP 响应对象
        static void save_file(const std::string &dst_path, const std::string &content, HttpResp *resp);

        // 保存内容到文件中，并将结果反馈到 HTTP 响应对象，同时携带通知消息
        // 参数：
        // - dst_path: 目标文件路径
        // - content: 要保存的内容
        // - resp: HTTP 响应对象
        // - notify_msg: 通知消息
        static void save_file(const std::string &dst_path, const std::string &content, 
                                            HttpResp *resp, const std::string &notify_msg);

        // 保存内容到文件中，并将结果反馈到 HTTP 响应对象，同时提供文件 I/O 回调函数
        // 参数：
        // - dst_path: 目标文件路径
        // - content: 要保存的内容
        // - resp: HTTP 响应对象
        // - func: 文件 I/O 回调函数
        static void save_file(const std::string &dst_path, const std::string &content, 
                          HttpResp *resp, const FileIOArgsFunc &func);

        // 保存内容到文件中，并将结果反馈到 HTTP 响应对象（使用右值引用优化性能）
        // 参数：
        // - dst_path: 目标文件路径
        // - content: 要保存的内容（右值引用）
        // - resp: HTTP 响应对象
        static void save_file(const std::string &dst_path, std::string&& content, HttpResp *resp);

        // 保存内容到文件中，并将结果反馈到 HTTP 响应对象，同时携带通知消息（使用右值引用优化性能）
        // 参数：
        // - dst_path: 目标文件路径
        // - content: 要保存的内容（右值引用）
        // - resp: HTTP 响应对象
        // - notify_msg: 通知消息
        static void save_file(const std::string &dst_path, std::string&& content, 
                                            HttpResp *resp, const std::string &notify_msg);

        // 保存内容到文件中，并将结果反馈到 HTTP 响应对象，同时提供文件 I/O 回调函数（使用右值引用优化性能）
        // 参数：
        // - dst_path: 目标文件路径
        // - content: 要保存的内容（右值引用）
        // - resp: HTTP 响应对象
        // - func: 文件 I/O 回调函数
        static void save_file(const std::string &dst_path, std::string &&content, 
                          HttpResp *resp, const FileIOArgsFunc &func);

    private:
        // 私有实现：保存内容到文件中，并将结果反馈到 HTTP 响应对象，同时携带通知消息和文件 I/O 回调函数
        // 参数：
        // - dst_path: 目标文件路径
        // - content: 要保存的内容
        // - resp: HTTP 响应对象
        // - notify_msg: 通知消息
        // - func: 文件 I/O 回调函数
        static void save_file(const std::string &dst_path, const std::string &content, 
                          HttpResp *resp, const std::string &notify_msg, 
                          const FileIOArgsFunc &func);

        // 私有实现：保存内容到文件中，并将结果反馈到 HTTP 响应对象，同时携带通知消息和文件 I/O 回调函数（使用右值引用优化性能）
        // 参数：
        // - dst_path: 目标文件路径
        // - content: 要保存的内容（右值引用）
        // - resp: HTTP 响应对象
        // - notify_msg: 通知消息
        // - func: 文件 I/O 回调函数
        static void save_file(const std::string &dst_path, std::string &&content, 
                          HttpResp *resp, const std::string &notify_msg, 
                          const FileIOArgsFunc &func);
    };

}  // namespace Yukino

#endif // YUKINO_HTTPFILE_H_