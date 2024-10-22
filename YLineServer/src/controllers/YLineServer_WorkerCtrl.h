#pragma once

#include <drogon/WebSocketController.h>
#include <entt/entt.hpp>

using namespace drogon;

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
    entt::registry workerRegistry;
};
}
