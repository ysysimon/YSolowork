#include "AMQP/AMQPconnectionPool.h"
#include "spdlog/spdlog.h"
#include <cstddef>
#include <memory>

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

        auto amqpHandler = TrantorHandler::create
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

std::unique_ptr<AMQP::Channel>
AMQPConnectionPool::make_channel()
{
    static std::atomic<size_t> index = 0; // 使用原子变量保证线程安全
    size_t handlerIndex = index.fetch_add(1, std::memory_order_relaxed) % m_AMQPHandler.size();
    spdlog::debug("Try to create AMQP Channel comes from Handler {0} 尝试创建 AMQP 通道来自 Handler {0}", handlerIndex);
    
    // 获取 AMQP 连接
    auto connection = m_AMQPHandler[handlerIndex]->getAMQPConnection();
    if (!connection)
    {
        spdlog::error
        (
            "AMQP Channel create failed Connection is not available 通道创建失败, AMQP 连接不可用"
        );
        return nullptr; // 返回空指针
    }

    // 创建 AMQP 通道
    auto new_channel = std::make_unique<AMQP::Channel>(connection);

    // 设置错误回调
    new_channel->onError
    (
        [this, handlerIndex](const char *message)
        {
            spdlog::error
            (
                "AMQP Channel comes from Handler {0} has error: {1} 来自 Handler {0} 的 AMQP 通道发生错误: {1}",
                handlerIndex,
                message
            ); 
        }
    );

    return new_channel;
}


}// namespace YLineServer