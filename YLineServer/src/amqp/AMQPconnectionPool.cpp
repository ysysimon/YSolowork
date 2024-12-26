#include "amqp/AMQPconnectionPool.h"

namespace YLineServer
{

AMQPConnectionPool::AMQPConnectionPool(
    const std::string &host,
    const uint16_t port,
    const std::string &username,
    const std::string &password
): m_host(host), m_port(port)
{
    auto threadNum = drogon::app().getThreadNum();
    for (size_t i = 0; i < threadNum; ++i)
    {
        // AMQP 连接
        auto loop = drogon::app().getIOLoop(i);
        auto amqpHandler = std::make_shared<TrantorHandler>
        (
            std::format("RabbitMQTCP-{}", i),
            loop,
            m_host,
            m_port,
            username,
            password
        );

        // 将 AMQP 服务器 TCP 连接 注册到事件循环
        amqpHandler->connect();

        m_AMQPHandler.push_back(amqpHandler);
    }
}

}// namespace YLineServer