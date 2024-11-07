#pragma once

#include "json/config.h"
#include <drogon/WebSocketController.h>

using namespace drogon;

namespace YLineServer
{
class WorkerStatusCtrl : public drogon::WebSocketController<WorkerStatusCtrl>
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
    WS_PATH_ADD(
      "/ws/worker_status"
      // Ufortunately, we cannot use filters in WebSocket, because WebBrowser's javascript WebSocket API does not support custom headers
      // "YLineServer::LoginFilter"
    );
    WS_PATH_LIST_END
  private:

  // Commands
  enum class CommandType {
    auth,
    requireWorkers,
    UNKNOWN  // 用于处理未识别的指令
  };

  // Command Map
  inline static std::unordered_map<std::string, CommandType> commandMap = {
    {"auth", CommandType::auth},
    {"requireWorkers", CommandType::requireWorkers}
  };

  // Command Fucntions
  void CommandsetWorkerCount(const WebSocketConnectionPtr& wsConnPtr);
  void CommandrequireWorkerInfo(const WebSocketConnectionPtr& wsConnPtr, const Json::Int64 fist, const Json::Int64 last);
    
}; // class WorkerStatusCtrl




} // namespace YLineServer
