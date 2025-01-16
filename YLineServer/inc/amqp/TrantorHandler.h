#ifndef YLINE_AMQP_TRANTOR_H
#define YLINE_AMQP_TRANTOR_H

#include "trantor/net/TcpClient.h"
#include <amqpcpp/include/amqpcpp.h>

#include <memory>
#include <trantor/net/EventLoop.h>
#include <trantor/net/Channel.h>

#include <spdlog/spdlog.h>

#include <entt/signal/sigh.hpp>

namespace YLineServer{

class TrantorHandler
// std::enable_shared_from_this<T> 是一个标准库提供的工具类
// 允许 std::shared_ptr 管理的对象在其内部安全地生成 std::shared_ptr 的实例
    : public AMQP::ConnectionHandler, public std::enable_shared_from_this<TrantorHandler>
{
public:
    ~TrantorHandler() override = default;

    inline static std::shared_ptr<TrantorHandler> create 
    (
        const std::string & name, 
        trantor::EventLoop * loop, 
        const std::string & host, 
        const uint16_t port,
        const std::string & username,
        const std::string & password
    )
    {
        auto instance = std::shared_ptr<TrantorHandler>
        (
            new TrantorHandler(name, loop)
        );
        instance->setupTcpClient(instance->m_loop, host, port, username, password);
        return instance;
    };

    inline void
    onError(AMQP::Connection *connection, const char *message) override
    {
        // spdlog::error("{} AMQP error 错误: {}", m_name, message);
        // spdlog::info("test state: {}",connection->usable());
    }

    inline void
    onClosed(AMQP::Connection *connection) override 
    {
        // spdlog::debug("{} connection closed 连接已关闭", m_name);
    }

    inline void
    connect()
    {
        m_tcpClient->connect();
    }

    void
    onData(AMQP::Connection *connection, const char *data, size_t size) override;

    void 
    onReady(AMQP::Connection *connection) override;

    inline AMQP::Connection * const
    getAMQPConnection() const
    {
        return _amqpConnection.get();
    }

    inline entt::sigh<void()> &
    getRecoonectSignal()
    {
        return m_onReconnect;
    }

private:
    std::shared_ptr<trantor::TcpClient> m_tcpClient;
    std::unique_ptr<AMQP::Connection> _amqpConnection;
    std::string m_name;
    trantor::EventLoop * m_loop;
    entt::sigh<void()> m_onReconnect;

    explicit inline
    TrantorHandler(
        const std::string & name,
        trantor::EventLoop * loop
    ) : m_name(name), m_loop(loop) {};

    void
    setConnection(
        const trantor::TcpConnectionPtr& conn,
        const std::string &username,
        const std::string &password
    );

    void
    setupTcpClient
    (
        trantor::EventLoop * loop, 
        const std::string & host, 
        const uint16_t port,
        const std::string & username,
        const std::string & password
    );

    void
    reConnectTcpClient
    (
        const std::weak_ptr<TrantorHandler> weakPtr,
        const std::string & username,
        const std::string & password,
        const std::string & host,
        const uint16_t port
    );

}; // class TrantorHandler

} // namespace YLineServer

#endif // YLINE_AMQP_TRANTOR_H

/*
通过 Trantor 发起和管理 TCP 连接，然后将接收到的 TCP 数据 交给 AMQP-CPP 进行 AMQP 协议解析

RabbitMQ <----> Trantor::TcpClient <----> Trantor::TcpConnection
                          |                       |
                   setRecvMsgCallback           send()
                          |                       |
                          v                       ^
                AMQP::Connection::parse      AMQP::ConnectionHandler::onData
                          |
           AMQP 协议处理，触发回调 (onReady, onClosed, onError 等)

*/