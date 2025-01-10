#ifndef YLINESERVER_SERVER_H
#define YLINESERVER_SERVER_H

#include "trantor/net/EventLoopThread.h"
#include "utils/config.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <entt/entt.hpp>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <shared_mutex>
#include <vector>
#include "drogon/WebSocketConnection.h"
#include "models/Users.h"

#include "amqp/AMQPconnectionPool.h"

using EnTTidType = entt::registry::entity_type;
using namespace drogon;
using namespace drogon_model::yline;

namespace YLineServer {


// 定义跨平台的自定义哈希函数
struct UuidHash {
    std::size_t operator()(const boost::uuids::uuid& uuid) const noexcept {
        return boost::uuids::hash_value(uuid); // 使用 boost 提供的 hash_value 函数
    }
};


// 单例类: 配置 config
class ServerSingleton {
public:
    // 获取单例实例的静态方法
    static inline ServerSingleton& getInstance()
    {
        static ServerSingleton instance; // 单例实例
        return instance;
    }

    // 禁止复制和赋值
    ServerSingleton(const ServerSingleton&) = delete;
    ServerSingleton& operator=(const ServerSingleton&) = delete;

    // 获取配置数据
    inline const Config& getConfigData() const
    {
        return configData_;
    }

    // 设置配置数据
    inline void setConfigData(const Config& configData)
    {
        configData_ = configData;
    }

    // 获取服务器实例 UUID
    inline boost::uuids::uuid getServerInstanceUUID() const {
        return server_instance_uuid;
    }

    

    entt::registry Registry; // EnTT registry 全局注册表

    // 添加工作机到 EnTT
    EnTTidType addUserEntt(const Users::PrimaryKeyType& userId, const std::string& username, bool isAdmin = false);

    // 工作机 UUID 到 EnTTid 映射
    std::unordered_map<boost::uuids::uuid, EnTTidType, UuidHash> WorkerUUIDtoEnTTid; 

    // hash 表，存储 WebSocket 连接到工作机 UUID 的映射 
    std::unordered_map<drogon::WebSocketConnectionPtr, boost::uuids::uuid> wsConnToWorkerUUID;

    // hash 表，存储用户名到 EnTTid 的映射
    std::unordered_map<std::string, EnTTidType> usernameToEnTTid; // 用户名到 EnTTid 映射

    // 共享锁
    std::shared_mutex workerUUIDMapMutex;
    std::shared_mutex wsConnMapMutex;

    // AMQP 连接池 for Producer
    std::shared_ptr<AMQPConnectionPool> amqpConnectionPool;

    // 消费者线程
    std::shared_ptr<trantor::EventLoopThread> consumerLoopThread;

    // 消费者 I/O 线程
    std::shared_ptr<trantor::EventLoopThread> consumerLoopIOThread;

    // AMQP 连接池 for Consumer
    std::shared_ptr<AMQPConnectionPool> consumer_amqpConnectionPool;
private:
    inline ServerSingleton()  // 私有构造函数，防止外部实例化
        : server_instance_uuid(boost::uuids::random_generator()())
    {
        spdlog::info("Server Instance UUID: {}", boost::uuids::to_string(server_instance_uuid));
    }

    Config configData_; // 成员变量，存储配置信息

    boost::uuids::uuid server_instance_uuid; // 服务器实例 UUID
};

// EnTT Components
// Since ORM APIs usually update objects as a whole, 
// we also store the entity components as a whole and do not split them up for now.

struct Worker {
    boost::uuids::uuid worker_uuid;
    boost::uuids::uuid server_instance_uuid;
    Json::Value worker_info;
    WebSocketConnectionPtr wsConnPtr;
};
namespace Components {



// decided to remove this component
// struct WSConnection {
//     WebSocketConnectionPtr wsConnPtr;
// };

struct User {
    Users::PrimaryKeyType userId;
    std::string username;
    bool isAdmin;
    std::vector<WebSocketConnectionPtr> wsConnPtrs;
};


} // namespace Components

} // namespace YLineServer


#endif // YLINESERVER_SERVER_H