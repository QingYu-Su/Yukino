#include "workflow/WFTaskFactory.h"
#include <sys/stat.h>
#include "HttpFile.h"
#include "HttpMsg.h"
#include "PathUtil.h"
#include "HttpServerTask.h"
#include "FileUtil.h"
#include "ErrorCode.h"

namespace Yukino
{
namespace
{
    /**
     * @brief 上下文结构，用于保存文件操作的相关信息
     */
    struct SaveFileContext
    {
        std::string content; // 要保存的文件内容
        std::string notify_msg; // 操作完成后的通知消息
        HttpFile::FileIOArgsFunc fileio_args_func; // 文件 I/O 操作的回调函数
    };

    /**
     * @brief 异步文件读取任务的回调函数
     * 
     * @param pread_task 异步文件读取任务对象
     */
    void pread_callback(WFFileIOTask *pread_task)
    {
        FileIOArgs *args = pread_task->get_args(); // 获取任务的参数
        long ret = pread_task->get_retval(); // 获取任务的返回值
        auto *resp = static_cast<HttpResp *>(pread_task->user_data); // 获取用户数据（HttpResp 对象）

        // 如果任务失败或返回值小于 0，表示文件读取失败
        if (pread_task->get_state() != WFT_STATE_SUCCESS || ret < 0)
        {
            resp->Error(StatusFileReadError); // 设置错误状态码并返回
        } 
        else
        {
            // 将读取到的数据追加到响应体中
            resp->append_output_body_nocopy(args->buf, ret);
        }
    }

    /**
     * @brief 异步文件写入任务的回调函数
     * 
     * @param pwrite_task 异步文件写入任务对象
     */
    void pwrite_callback(WFFileIOTask *pwrite_task)
    {
        long ret = pwrite_task->get_retval(); // 获取任务的返回值
        HttpServerTask *server_task = task_of(pwrite_task); // 获取 HttpServerTask 对象
        HttpResp *resp = server_task->get_resp(); // 获取 HttpResp 对象
        auto *save_context = static_cast<SaveFileContext *>(pwrite_task->user_data); // 获取用户数据（SaveFileContext 对象）

        // 如果指定了文件 I/O 回调函数，则调用它
        if(save_context->fileio_args_func)
        {
            save_context->fileio_args_func(pwrite_task->get_args());
        }

        // 如果任务失败或返回值小于 0，表示文件写入失败
        if (pwrite_task->get_state() != WFT_STATE_SUCCESS || ret < 0)
        {
            resp->Error(StatusFileWriteError); // 设置错误状态码并返回
        } 
        else
        {
            // 如果指定了通知消息，则将通知消息追加到响应体中
            if(!save_context->notify_msg.empty())
            {
                resp->append_output_body_nocopy(save_context->notify_msg.c_str(), save_context->notify_msg.size());
            }
        }
    }

}  // namespace

// note : [start, end)
int HttpFile::send_file(const std::string &path, size_t file_start, size_t file_end, HttpResp *resp)
{
    // 检查文件是否存在
    if (!PathUtil::is_file(path))
    {
        return StatusNotFound; // 如果文件不存在，返回状态码 StatusNotFound
    }

    // 初始化文件范围的起始和结束位置
    int start = file_start;
    int end = file_end;

    // 如果结束位置为 -1 或起始位置小于 0，需要获取文件大小并调整范围
    if (end == -1 || start < 0)
    {
        size_t file_size;
        int ret = FileUtil::size(path, &file_size); // 获取文件大小

        if (ret != StatusOK)
        {
            return ret; // 如果获取文件大小失败，返回错误状态码
        }

        if (end == -1) end = file_size; // 如果结束位置为 -1，设置为文件大小
        if (start < 0) start = file_size + start; // 如果起始位置为负数，计算从文件末尾开始的位置
    }

    // 检查文件范围是否有效
    if (end <= start)
    {
        return StatusFileRangeInvalid; // 如果结束位置小于或等于起始位置，返回状态码 StatusFileRangeInvalid
    }

    // 确定文件的内容类型
    http_content_type content_type = CONTENT_TYPE_NONE;
    std::string suffix = PathUtil::suffix(path); // 获取文件扩展名
    if (!suffix.empty())
    {
        content_type = ContentType::to_enum_by_suffix(suffix); // 根据扩展名获取内容类型
    }
    if (content_type == CONTENT_TYPE_NONE || content_type == CONTENT_TYPE_UNDEFINED)
    {
        content_type = APPLICATION_OCTET_STREAM; // 如果无法确定内容类型，设置为默认的二进制流类型
    }
    resp->headers["Content-Type"] = ContentType::to_str(content_type); // 设置响应头中的 Content-Type

    // 计算要读取的文件片段大小
    size_t size = end - start;
    void *buf = malloc(size); // 分配内存用于存储文件片段

    // 获取当前的 HttpServerTask 对象
    HttpServerTask *server_task = task_of(resp);

    // 添加回调函数，用于在任务完成后释放分配的内存
    server_task->add_callback([buf](HttpTask *server_task)
                              {
                                  free(buf); // 释放内存
                              });

    // 设置响应头中的 Content-Range
    // https://datatracker.ietf.org/doc/html/rfc7233#section-4.2
    // Content-Range: bytes 42-1233/1234
    resp->headers["Content-Range"] = "bytes " + std::to_string(start)
                                            + "-" + std::to_string(end)
                                            + "/" + std::to_string(size);

    // 创建异步文件读取任务
    WFFileIOTask *pread_task = WFTaskFactory::create_pread_task(path,
                                                                buf,
                                                                size,
                                                                static_cast<off_t>(start),
                                                                pread_callback);
    pread_task->user_data = resp; // 设置用户数据为 HttpResp 对象

    // 将文件读取任务添加到服务器任务中
    **server_task << pread_task;

    return StatusOK; // 返回状态码 StatusOK
}


// 保存文件内容到指定路径（使用左值引用）
void HttpFile::save_file(const std::string &dst_path, const std::string &content,
                        HttpResp *resp, const std::string &notify_msg,
                        const FileIOArgsFunc &func)
{
    // 获取当前的 HttpServerTask 对象
    HttpServerTask *server_task = task_of(resp);

    // 创建一个 SaveFileContext 对象，用于保存文件操作的相关信息
    auto *save_context = new SaveFileContext;
    save_context->content = content;    // 复制内容
    save_context->notify_msg = notify_msg;  // 复制通知消息
    if (func)
    {
        save_context->fileio_args_func = func; // 设置文件 I/O 回调函数
    }

    // 创建异步文件写入任务
    WFFileIOTask *pwrite_task = WFTaskFactory::create_pwrite_task(
        dst_path, // 目标文件路径
        static_cast<const void *>(save_context->content.c_str()), // 文件内容
        save_context->content.size(), // 文件内容大小
        0, // 写入偏移量
        pwrite_callback // 回调函数
    );

    // 将文件写入任务添加到服务器任务中
    **server_task << pwrite_task;

    // 添加回调函数，用于在任务完成后释放 SaveFileContext 对象
    server_task->add_callback([save_context](HttpTask *) {
        delete save_context;
    });

    // 设置用户数据为 SaveFileContext 对象
    pwrite_task->user_data = save_context;
}

// 保存文件内容到指定路径（使用右值引用优化性能）
void HttpFile::save_file(const std::string &dst_path, std::string &&content,
                        HttpResp *resp, const std::string &notify_msg,
                        const FileIOArgsFunc &func)
{
    // 获取当前的 HttpServerTask 对象
    HttpServerTask *server_task = task_of(resp);

    // 创建一个 SaveFileContext 对象，用于保存文件操作的相关信息
    auto *save_context = new SaveFileContext;
    save_context->content = std::move(content); // 使用右值引用移动内容，避免拷贝
    save_context->notify_msg = std::move(notify_msg); // 使用右值引用移动通知消息，避免拷贝
    if (func)
    {
        save_context->fileio_args_func = func; // 设置文件 I/O 回调函数
    }

    // 创建异步文件写入任务
    WFFileIOTask *pwrite_task = WFTaskFactory::create_pwrite_task(
        dst_path, // 目标文件路径
        static_cast<const void *>(save_context->content.c_str()), // 文件内容
        save_context->content.size(), // 文件内容大小
        0, // 写入偏移量
        pwrite_callback // 回调函数
    );

    // 将文件写入任务添加到服务器任务中
    **server_task << pwrite_task;

    // 添加回调函数，用于在任务完成后释放 SaveFileContext 对象
    server_task->add_callback([save_context](HttpTask *) {
        delete save_context;
    });

    // 设置用户数据为 SaveFileContext 对象
    pwrite_task->user_data = save_context;
}

// 保存文件内容到指定路径，不提供通知消息和回调函数
void HttpFile::save_file(const std::string &dst_path, const std::string &content, HttpResp *resp)
{
    return save_file(dst_path, content, resp, "", nullptr);
}

// 保存文件内容到指定路径，提供通知消息，但不提供回调函数
void HttpFile::save_file(const std::string &dst_path, const std::string &content,
                                HttpResp *resp, const std::string &notify_msg)
{
    return save_file(dst_path, content, resp, notify_msg, nullptr);
}

// 保存文件内容到指定路径，提供回调函数，但不提供通知消息
void HttpFile::save_file(const std::string &dst_path, const std::string &content,
                HttpResp *resp, const FileIOArgsFunc &func)
{
    return save_file(dst_path, content, resp, "", func);
}

// 保存文件内容到指定路径（使用右值引用优化性能），不提供通知消息和回调函数
void HttpFile::save_file(const std::string &dst_path, std::string&& content, HttpResp *resp)
{
    return save_file(dst_path, std::move(content), resp, "", nullptr);
}

// 保存文件内容到指定路径（使用右值引用优化性能），提供通知消息，但不提供回调函数
void HttpFile::save_file(const std::string &dst_path, std::string&& content,
                                HttpResp *resp, const std::string &notify_msg)
{
    return save_file(dst_path, std::move(content), resp, notify_msg, nullptr);
}

// 保存文件内容到指定路径（使用右值引用优化性能），提供回调函数，但不提供通知消息
void HttpFile::save_file(const std::string &dst_path, std::string &&content,
                HttpResp *resp, const FileIOArgsFunc &func)
{
    return save_file(dst_path, std::move(content), resp, "", func);
}

}  // namespace Yukino
