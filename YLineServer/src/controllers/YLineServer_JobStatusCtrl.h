#pragma once

#include <drogon/WebSocketController.h>

using namespace drogon;

namespace YLineServer
{
class JobStatusCtrl : public drogon::WebSocketController<JobStatusCtrl>
{
  public:
     void handleNewMessage(const WebSocketConnectionPtr&,
                                  std::string &&,
                                  const WebSocketMessageType &) override;
    void handleNewConnection(const HttpRequestPtr &,
                                     const WebSocketConnectionPtr&) override;
    void handleConnectionClosed(const WebSocketConnectionPtr&) override;
    WS_PATH_LIST_BEGIN
    WS_PATH_ADD(
      "/ws/job_status"
    );
    WS_PATH_LIST_END

  private:
  // Commands
  enum class CommandType {
    auth,
    requireJobs,
    requireJobStatus,
    UNKNOWN  // 用于处理未识别的指令
  };

  // Command Map
  inline static std::unordered_map<std::string, CommandType> commandMap = {
    {"auth", CommandType::auth},
    {"requireJobs", CommandType::requireJobs},
    {"requireJobStatus", CommandType::requireJobStatus}
  };

  // Command Fucntions
  void 
  CommandsetJobCount(const WebSocketConnectionPtr& wsConnPtr);

  void
  CommandrequireJobInfo(const WebSocketConnectionPtr& wsConnPtr, const Json::Int64 fist, const Json::Int64 last);

  void
  CommandrequireJobStatus(const WebSocketConnectionPtr& wsConnPtr, const Json::Int64 job_id);

};
}
