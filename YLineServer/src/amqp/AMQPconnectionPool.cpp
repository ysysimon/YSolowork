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
        auto *loop = drogon::app().getIOLoop(i);
        auto amqpHandler = std::make_shared<TrantorHandler>(
            std::format("RabbitMQTCP-{}", i),
            loop,
            m_host,
            m_port,
            username,
            password
        );

        m_AMQPConnections.push_back(amqpHandler);

    }
}

}// namespace YLineServer