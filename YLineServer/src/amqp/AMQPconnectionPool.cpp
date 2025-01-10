#include "AMQP/AMQPconnectionPool.h"
#include "spdlog/spdlog.h"
#include <cstddef>
#include <memory>

namespace YLineServer
{

AMQPConnectionPool::AMQPConnectionPool
(
    const std::string & host,
    const uint16_t port,
    const std::string & username,
    const std::string & password,
    const std::string & pool_name
): m_host(host), m_port(port), m_pool_name(pool_name)
{
    auto threadNum = drogon::app().getThreadNum();
    for (size_t i = 0; i < threadNum; ++i)
    {
        // AMQP 连接
        auto loop = drogon::app().getIOLoop(i);

        auto amqpHandler = TrantorHandler::create
        (
            std::format("{} conneciton - {}", m_pool_name, i),
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

AMQPConnectionPool::AMQPConnectionPool
(
    const std::string & host,
    const uint16_t port,
    const std::string & username,
    const std::string & password,
    trantor::EventLoop * loop,
    const std::uint32_t connectionNum,
    const std::string & pool_name
) : m_host(host), m_port(port), m_pool_name(pool_name)
{
    for (std::size_t i = 0; i < connectionNum; ++i)
    {
        // AMQP 连接
        auto amqpHandler = TrantorHandler::create
        (
            std::format("{} conneciton - {}", m_pool_name, i),
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
    spdlog::debug("Pool `{0}` Try to create AMQP Channel comes from Handler {1} 尝试创建 AMQP 通道来自 Handler {1}", m_pool_name, handlerIndex);
    
    // 获取 AMQP 连接
    auto connection = m_AMQPHandler[handlerIndex]->getAMQPConnection();
    if (!connection)
    {
        spdlog::error
        (
            "Pool `{0}` AMQP Channel create failed, Connection is not available 通道创建失败, AMQP 连接不可用",
            m_pool_name
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
                "Pool `{0}` AMQP Channel comes from Handler {1} has error: {2} 来自 Handler {1} 的 AMQP 通道发生错误: {2}",
                m_pool_name,
                handlerIndex,
                message
            ); 
        }
    );

    return new_channel;
}


}// namespace YLineServer