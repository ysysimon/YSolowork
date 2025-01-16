#ifndef YLineServer_CONSUMER_H
#define YLineServer_CONSUMER_H

#include "utils/config.h"
#include "AMQP/AMQPconnectionPool.h"

namespace Queue
{
    const std::string default_queue = "default";
}

namespace YLineServer::Components
{
struct Consumer
{
    explicit inline
    Consumer(entt::sigh<void()> & signal, std::unique_ptr<AMQP::Channel> & channel, const std::string & queueName = Queue::default_queue)
        :m_queueName(queueName), m_signal(signal), m_channel(std::move(channel)), m_onReconnect(m_signal)
    {
        createChannel();
    }

    void // 重建 Channel 的函数
    rebuildChannel();
private:
    std::unique_ptr<AMQP::Channel> m_channel;
    std::shared_ptr<int> lifecycleHelper;
    std::string m_queueName;
    entt::sigh<void()> & m_signal;
    entt::sink<entt::sigh<void()>> m_onReconnect;

    void // 创建 Channel 的函数
    createChannel();
};

} // namespace YLineServer::Components


namespace YLineServer::task
{

void // 初始化消费者线程
initConsumerLoop(const Config & config);

Components::Consumer // 创建消费者
make_Consumer(const AMQPConnectionPool& pool, const std::string & queueName = Queue::default_queue);


} // namespace YLineServer::task

#endif // YLineServer_CONSUMER_H