#ifndef YUKINO_HTTPSERVERTASK_H_
#define YUKINO_HTTPSERVERTASK_H_

#include "HttpMsg.h"
#include "Noncopyable.h"

namespace Yukino
{

class HttpServer;

/**
 * @brief HttpServerTask 类，表示 HTTP 服务器任务
 * 
 * 该类继承自 WFServerTask<HttpReq, HttpResp> 并且是不可拷贝的（继承自 Noncopyable）。
 * 它封装了 HTTP 服务器任务的处理逻辑，包括请求处理、回调函数管理等。
 * WFServerTask<HttpReq, HttpResp>继承了WFNetworkTask<HttpReq, HttpResp>，即HttpTask的子类
 * 那么HttpServerTask也是HttpTask的子类
 */
class HttpServerTask : public WFServerTask<HttpReq, HttpResp> , public Noncopyable
{
public:
    // 定义处理函数类型
    using ProcFunc = std::function<void(HttpTask *)>;
    // 定义服务器回调函数类型
    using ServerCallBack = std::function<void(HttpTask *)>;

    // 使用 WFServerTask 的 Series 类型
    using WFServerTask::Series;

    // 声明 HttpServer 为友元类
    friend class HttpServer;

    /**
     * @brief 构造函数
     * 
     * @param service 通信服务对象
     * @param process 请求处理函数
     */
    HttpServerTask(CommService *service, ProcFunc &process);

    /**
     * @brief 添加回调函数
     * 
     * @param cb 回调函数
     */
    void add_callback(const ServerCallBack &cb)
    { cb_list_.push_back(cb); }

    /**
     * @brief 添加回调函数（右值引用优化）
     * 
     * @param cb 回调函数
     */
    void add_callback(ServerCallBack &&cb)
    { cb_list_.emplace_back(std::move(cb)); }

    /**
     * @brief 获取响应对象的偏移量
     * 
     * @return size_t 响应对象的偏移量
     */
    static size_t get_resp_offset()
    {
        HttpServerTask task(nullptr);
        return task.resp_offset();
    }

    /**
     * @brief 获取对端地址
     * 
     * @return std::string 对端地址
     */
    std::string peer_addr() const;

    /**
     * @brief 获取对端端口
     * 
     * @return unsigned short 对端端口
     */
    unsigned short peer_port() const;

    /**
     * @brief 获取关闭标志
     * 
     * @return bool 是否关闭
     */
    bool close_flag() const;

protected:
    /**
     * @brief 处理任务状态
     * 
     * @param state 任务状态
     * @param error 错误码
     */
    void handle(int state, int error) override;

    /**
     * @brief 获取消息输出对象
     * 
     * @return CommMessageOut* 消息输出对象
     */
    CommMessageOut *message_out() override;

private:
    /**
     * @brief 隐藏 set_callback 方法
     */
    void set_callback()
    {}

    /**
     * @brief 获取响应对象的偏移量
     * 
     * @return size_t 响应对象的偏移量
     */
    size_t resp_offset() const
    {
        return (const char *) (&this->resp) - (const char *) this;
    }

    /**
     * @brief 构造函数（仅用于 get_resp_offset）
     * 
     * @param proc 请求处理函数
     */
    HttpServerTask(std::function<void(HttpTask *)> proc) :
            WFServerTask(nullptr, nullptr, proc)
    {}

private:
    bool req_is_alive_; // 请求是否存活
    bool req_has_keep_alive_header_; // 请求是否包含 Keep-Alive 头
    std::string req_keep_alive_; // Keep-Alive 头的值
    std::vector<ServerCallBack> cb_list_; // 回调函数列表
    HttpServer* server = nullptr; // 指向 HttpServer 的指针
};

/**
 * @brief 从子任务获取 HttpServerTask 对象
 * 
 * @param task 子任务对象
 * @return HttpServerTask* HttpServerTask 对象
 */
inline HttpServerTask *task_of(const SubTask *task)
{
    auto *series = static_cast<HttpServerTask::Series *>(series_of(task));
    return static_cast<HttpServerTask *>(series->task);
}

/**
 * @brief 从 HttpResp 对象获取 HttpServerTask 对象
 * 
 * @param resp HttpResp 对象
 * @return HttpServerTask* HttpServerTask 对象
 */
inline HttpServerTask *task_of(const HttpResp *resp)
{
    size_t http_resp_offset = HttpServerTask::get_resp_offset();
    return (HttpServerTask *) ((char *) (resp) - http_resp_offset);
}

} // namespace Yukino

#endif // YUKINO_HTTPSERVERTASK_H_