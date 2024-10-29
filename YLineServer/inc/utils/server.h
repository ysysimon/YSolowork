#ifndef YLINESERVER_SERVER_H
#define YLINESERVER_SERVER_H

#include "config.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <entt/entt.hpp>
#include <spdlog/spdlog.h>
#include <unordered_map>

using EnTTidType = entt::registry::entity_type;

namespace YLineServer {

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

    std::unordered_map<boost::uuids::uuid, EnTTidType> WorkerUUIDtoEnTTid; // 工作机 UUID 到 EnTTid 映射

private:
    inline ServerSingleton()  // 私有构造函数，防止外部实例化
        : server_instance_uuid(boost::uuids::random_generator()())
    {
        spdlog::info("Server Instance UUID: {}", boost::uuids::to_string(server_instance_uuid));
    }

    Config configData_; // 成员变量，存储配置信息

    boost::uuids::uuid server_instance_uuid; // 服务器实例 UUID
};


}


#endif // YLINESERVER_SERVER_H