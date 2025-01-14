#ifndef YLineServer_Componets_WORKER_H
#define YLineServer_Componets_WORKER_H

#include <boost/uuid/uuid.hpp>
#include "drogon/WebSocketConnection.h"

#include <amqpcpp/include/amqpcpp.h>
#include <memory>
#include "AMQP/AMQPconnectionPool.h"// get default_queue name
#include "trantor/net/EventLoop.h"



namespace YLineServer::Components
{

struct Worker {
    boost::uuids::uuid worker_uuid;
    boost::uuids::uuid server_instance_uuid;
    Json::Value worker_info;
    drogon::WebSocketConnectionPtr wsConnPtr;
};

struct Consumer
{
    explicit inline
    Consumer(const std::string & queueName = Queue::default_queue)
    {
        createChannel(channel, queueName);
    }

    ~Consumer();
private:
    std::unique_ptr<AMQP::Channel> channel;
    std::shared_ptr<int> lifecycleHelper;
    trantor::TimerId resume_timerId = trantor::InvalidTimerId;

    void // 重建 Channel 的函数
    rebuildChannel(std::unique_ptr<AMQP::Channel> & channel, const std::string & queueName);

    void // 创建 Channel 的函数
    createChannel(std::unique_ptr<AMQP::Channel> & channel, const std::string & queueName);

};

}; // namespace YLineServer::Components



#endif // YLineServer_Componets_WORKER_H