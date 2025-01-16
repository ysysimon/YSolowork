#ifndef YLINESERVER_AMQP_CONNECTION_POOL_H
#define YLINESERVER_AMQP_CONNECTION_POOL_H

#include <memory>
#include <string>
#include <vector>
#include <amqpcpp/include/amqpcpp.h>
#include "AMQP/TrantorHandler.h"
#include "trantor/net/EventLoop.h"


namespace YLineServer::Components
{
    // forward declaration
    struct Consumer;
}

namespace YLineServer
{
    // forward declaration
    class AMQPConnectionPool;
}

namespace YLineServer::task
{

Components::Consumer // 创建消费者
make_Consumer(const AMQPConnectionPool& pool, const std::string & queueName);

} // namespace YLineServer::task

namespace YLineServer
{

class AMQPConnectionPool
{
public:
    // 默认在 drogon 的所有 I/O 线程上创建 AMQP 连接
    explicit
    AMQPConnectionPool(
        const std::string & host,
        const uint16_t port,
        const std::string & username,
        const std::string & password,
        const std::string & pool_name
    );

    // 在指定的 EventLoop 上创建 AMQP 连接池
    explicit
    AMQPConnectionPool(
        const std::string & host,
        const uint16_t port,
        const std::string & username,
        const std::string & password,
        trantor::EventLoop * loop,
        const std::uint32_t connectionNum,
        const std::string & pool_name
    );

    ~AMQPConnectionPool() = default;
    
    inline const std::string&
    host() const { return m_host; }

    inline const uint16_t
    port() const { return m_port; }

    // rember to check if the return value is nullptr
    std::unique_ptr<AMQP::Channel>
    make_channel();

    bool
    ready() const;

    friend Components::Consumer // 创建消费者
    task::make_Consumer(const AMQPConnectionPool & pool, const std::string & queueName);
private:
    std::vector<std::shared_ptr<TrantorHandler>> m_AMQPHandler;
    std::string m_host;
    uint16_t m_port;
    std::string m_pool_name;
};

} // namespace YLineServer

#endif // YLINESERVER_AMQP_CONNECTION_POOL_H