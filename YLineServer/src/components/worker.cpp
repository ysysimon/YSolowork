#include "components/worker.h"
#include <memory>
#include <spdlog/spdlog.h>

#include "utils/server.h"

namespace YLineServer::Components
{

void // 重建 Channel 的函数
Consumer::createChannel()
{
    // 创建新的 Channel
    channel = YLineServer::ServerSingleton::getInstance().consumer_amqpConnectionPool->make_channel();

    // 设置消费者
    channel->consume(this->m_queueName)
        .onReceived([queueName = this->m_queueName](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {
            spdlog::info("Received message from queue `{0}`: {1}", queueName, std::string(message.body(), message.bodySize()));
        })
        .onSuccess([queueName = this->m_queueName]() {
            spdlog::info("Consumer for queue `{0}` has been started", queueName);
        });

    // 设置错误回调
    channel->onError
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
    
    createChannel();  // 重建 Channel
}

} // namespace YLineServer::Components