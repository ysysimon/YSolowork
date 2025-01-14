#include "components/worker.h"
#include <spdlog/spdlog.h>

#include "trantor/net/EventLoop.h"
#include "utils/server.h"

namespace YLineServer::Components
{

void // 重建 Channel 的函数
Consumer::createChannel(std::unique_ptr<AMQP::Channel> & channel, const std::string & queueName)
{
    // 创建新的 Channel
    channel = YLineServer::ServerSingleton::getInstance().consumer_amqpConnectionPool->make_channel();

    // 设置消费者
    channel->consume(queueName)
        .onReceived([queueName](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {
            spdlog::info("Received message from queue `{0}`: {1}", queueName, std::string(message.body(), message.bodySize()));
        })
        .onSuccess([queueName]() {
            spdlog::info("Consumer for queue `{0}` has been started", queueName);
        });

    // 设置错误回调
    channel->onError
    (
        [this, &channel, queueName](const char * message) 
        {
            spdlog::error
            (
                "Consumer for queue `{0}` has error: {1} 队列 `{0}` 的消费者发生错误: {1}",
                queueName,
                message
            );
            // 重建 Channel
            rebuildChannel(channel, queueName);
        }
    );

    // 注册一个检测
    auto weakSelf = weak_from_this();  // 捕获 weak_ptr
    resume_timerId = YLineServer::ServerSingleton::getInstance().consumerLoopThread->getLoop()->runEvery
    (
        1,
        [weakSelf, queueName]()
        {
            if(auto self = weakSelf.lock())
            {
                if (self->channel->usable())
                {
                    spdlog::warn("Channel for queue `{0}` is not usable, trying to rebuild 通道 `{0}` 不可用，尝试重建", queueName);
                    // 重建 Channel
                    self->rebuildChannel(self->channel, queueName);
                }
            }
            else 
            {
                spdlog::debug("Resume cancle, Consumer for queue `{0}` is destroyed 恢复取消, 消费者 `{0}` 已销毁", queueName);
            }
        }
    );
}

Consumer::~Consumer()
{
    // 取消定时器
    if (resume_timerId != trantor::InvalidTimerId)
    {
        YLineServer::ServerSingleton::getInstance().consumerLoopThread->getLoop()->invalidateTimer(resume_timerId);
    }
}

void
Consumer::rebuildChannel(std::unique_ptr<AMQP::Channel> & channel, const std::string & queueName)
{
    
    createChannel(channel, queueName);  // 重建 Channel
}

} // namespace YLineServer::Components