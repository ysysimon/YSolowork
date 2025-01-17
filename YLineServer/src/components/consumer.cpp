#include "components/consumer.h"
#include "utils/server.h"

namespace YLineServer::task
{
void 
initConsumerLoop(const Config & config)
{
    // 启动 消费者 I/O 线程 并连接属于消费者的连接池
    auto & consumerLoopIOThread = YLineServer::ServerSingleton::getInstance().consumerLoopIOThread;
    consumerLoopIOThread = std::make_shared<trantor::EventLoopThread>();
    consumerLoopIOThread->run(); // this won't block
    auto & amqpConnectionPoolConsumer = YLineServer::ServerSingleton::getInstance().consumer_amqpConnectionPool;
    auto consumerIOloop = consumerLoopIOThread->getLoop();
    consumerIOloop->queueInLoop
    (
        // 注意需要按引用捕获 amqpConnectionPool，因为上面的 shared_ptr 还未初始化，引用计数并未生效
        [ &config, &amqpConnectionPoolConsumer, consumerIOloop]() mutable
        {
            // 创建 AMQP 连接池
            amqpConnectionPoolConsumer = std::make_shared<YLineServer::AMQPConnectionPool>
            (
                config.amqp_host,
                config.amqp_port,
                config.amqp_user,
                config.amqp_password,
                consumerIOloop,
                config.consumer_AMQP_connection,
                "Consumer"
            );

            if (!amqpConnectionPoolConsumer)
            {
                throw std::runtime_error("AMQP Connection Pool creation for `Consumer` failed AMQP 消费者连接池创建失败");
            }

            spdlog::info("AMQP Connection Pool created for `Consumer` AMQP 消费者连接池已创建");
        }
    );

    
}

std::pair<AMQP::Connection *, entt::sigh<void()> &>
get_rebuild_Connection_and_signal(const AMQPConnectionPool & pool)
{
    static std::atomic<size_t> index = 0; // 使用原子变量保证线程安全
    size_t handlerIndex = index.fetch_add(1, std::memory_order_relaxed) % pool.m_AMQPHandler.size();
    spdlog::debug("Pool `{0}` Try to create AMQP Channel comes from Handler {1} 尝试创建 AMQP 通道来自 Handler {1}", pool.m_pool_name, handlerIndex);

    // 获取 AMQP 连接 和 重连信号
    auto connection = pool.m_AMQPHandler[handlerIndex]->getAMQPConnection();
    auto  & signal = pool.m_AMQPHandler[handlerIndex]->getRecoonectSignal();

    return std::make_pair(connection, std::ref(signal));
}

}; // namespace YLineServer::task

namespace YLineServer::Components
{

void // 重建 Channel 的函数
Consumer::createChannel()
{
    // 创建新的 Channel
    auto [connection, signal] = YLineServer::task::get_rebuild_Connection_and_signal(*YLineServer::ServerSingleton::getInstance().consumer_amqpConnectionPool);
    m_channel = std::make_unique<AMQP::Channel>(connection);
    m_onReconnect.disconnect(); // 断开之前的连接
    m_onReconnect = entt::sink {signal};
    m_onReconnect.connect<&Consumer::rebuildChannel>(*this); // 连接重建信号

    // 设置消费者
    m_channel->consume(this->m_queueName)
        .onReceived([queueName = this->m_queueName](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {
            spdlog::info("Received message from queue `{0}`: {1}", queueName, std::string(message.body(), message.bodySize()));
        })
        .onSuccess([queueName = this->m_queueName]() {
            spdlog::info("Consumer for queue `{0}` has been started", queueName);
        });

    // 设置错误回调
    m_channel->onError
    (
        [this, queueName = this->m_queueName](const char * message) 
        {
            spdlog::error
            (
                "Consumer for queue `{0}` has error: {1} 队列 `{0}` 的消费者发生错误: {1}",
                queueName,
                message
            );
            // 重建 Channel
            rebuildChannel();
        }
    );
}

void
Consumer::rebuildChannel()
{
    spdlog::warn("Rebuilding Channel 重建通道");
    createChannel();  // 重建 Channel
}

} // namespace YLineServer::Components
