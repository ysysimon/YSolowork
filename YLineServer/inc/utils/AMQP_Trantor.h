#ifndef YLINE_AMQP_TRANTOR_H
#define YLINE_AMQP_TRANTOR_H

#include <amqpcpp/include/amqpcpp.h>
#include <amqpcpp/include/amqpcpp/linux_tcp.h>

#include <trantor/net/EventLoop.h>
#include <trantor/net/Channel.h>

#include <spdlog/spdlog.h>

namespace YLineServer{

class TrantorHandler : public AMQP::TcpHandler 
{
public:
    explicit 
    TrantorHandler(trantor::EventLoop *loop) : _loop(loop) {}

    // 连接成功时的回调
    void inline
    onConnected(AMQP::TcpConnection *connection) override 
    {
        spdlog::info("AMQP connection established 连接建立成功");
    }

     // 连接关闭时的回调
    void inline 
    onClosed(AMQP::TcpConnection *connection) override 
    {
        spdlog::info("AMQP connection closed 连接关闭");
    }

    // 连接出错时的回调
    void inline
    onError(AMQP::TcpConnection *connection, const char *message) override 
    {
        spdlog::error("AMQP connection error 错误: {}", message);
    }

    // 注册或更新文件描述符上的事件
    void 
    monitor(AMQP::TcpConnection *connection, int fd, int flags) override;

private:
    // 处理事件
    void 
    handleEvent(const std::shared_ptr<trantor::Channel> &channel, int events);

    trantor::EventLoop *_loop;  // Drogon 的事件循环
    std::unordered_map<int, std::shared_ptr<trantor::Channel>> _channels;  // FD 到 Channel 的映射
    std::unordered_map<int, AMQP::TcpConnection *> _connections;  

}; // class TrantorHandler

} // namespace YLineServer

#endif // YLINE_AMQP_TRANTOR_H

/*
文件描述符（File Descriptor，FD）与 RabbitMQ 的连接有关，RabbitMQ 的 AMQP 通信协议依赖底层的 TCP 连接，在操作系统中，网络套接字（Socket）是通过文件描述符来表示的。
这是因为 AMQP 是一个应用层协议，其底层是基于 TCP 的二进制通信，每个 RabbitMQ 的连接（AMQP::TcpConnection）会使用一个 TCP 套接字，操作系统为这个套接字分配一个文件描述符，用于标识该连接，
应用程序需要使用文件描述符来读取和写入数据，以实现 AMQP 协议的数据交互
*/