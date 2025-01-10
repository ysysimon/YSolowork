#include "task/consumer.h"
#include "utils/server.h"

namespace YLineServer::task
{
void initConsumerLoop(const Config & config)
{
    // 启动 消费者 线程
    auto & consumerLoopThread = YLineServer::ServerSingleton::getInstance().consumerLoopThread;
    consumerLoopThread = std::make_shared<trantor::EventLoopThread>();
    consumerLoopThread->run(); // this won't block
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

}; // namespace YLineServer::task