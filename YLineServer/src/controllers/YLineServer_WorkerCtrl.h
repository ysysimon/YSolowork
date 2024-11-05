#pragma once

#include "drogon/WebSocketConnection.h"
#include "json/value.h"
#include <drogon/WebSocketController.h>
#include <entt/entt.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <unordered_map>

using namespace drogon;
using EnTTidType = entt::registry::entity_type;

namespace YLineServer
{
class WorkerCtrl : public drogon::WebSocketController<WorkerCtrl>
{
  public:
     void handleNewMessage(const WebSocketConnectionPtr&,
                                  std::string &&,
                                  const WebSocketMessageType &) override;
    void handleNewConnection(const HttpRequestPtr &,
                                     const WebSocketConnectionPtr&) override;
    void handleConnectionClosed(const WebSocketConnectionPtr&) override;
    WS_PATH_LIST_BEGIN
    // list path definitions here;
    // WS_PATH_ADD("/path", "filter1", "filter2", ...);
    WS_PATH_ADD("/ws/worker");
    WS_PATH_LIST_END
  
  private:
    void registerWorker(const std::string& workerUUID, const Json::Value& workerInfo, const WebSocketConnectionPtr& wsConnPtr) const;
    // void registerNewWorkerDatabase(const std::string& workerUUID, const Json::Value& workerInfo, EnTTidType workerEnTTid, const WebSocketConnectionPtr& wsConnPtr) const;
    // void registerWorkerEnTT(const std::string& workerUUID, const Json::Value& workerInfo, const WebSocketConnectionPtr& wsConnPtr) const;

    // command functions
    void writeUsage2redis(const Json::Value& usageJson, const WebSocketConnectionPtr& wsConnPtr) const;
};

// Commands
enum class CommandType {
    usage,
    UNKNOWN  // 用于处理未识别的指令
};

// Command Map
inline std::unordered_map<std::string, CommandType> commandMap = {
    {"usage", CommandType::usage}
};

} // YLineServer
