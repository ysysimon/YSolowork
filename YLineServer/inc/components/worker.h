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
        :m_queueName(queueName)
    {
        createChannel();
    }

    bool inline
    usable() const
    {
        if (channel && channel->usable())
        {
            return true;
        }

        return false;
    }

    void // 重建 Channel 的函数
    rebuildChannel();
private:
    std::unique_ptr<AMQP::Channel> channel;
    std::shared_ptr<int> lifecycleHelper;
    std::string m_queueName;

    void // 创建 Channel 的函数
    createChannel();

};

}; // namespace YLineServer::Components



#endif // YLineServer_Componets_WORKER_H