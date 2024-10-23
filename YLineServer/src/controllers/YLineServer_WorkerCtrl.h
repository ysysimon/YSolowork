#pragma once

#include "json/value.h"
#include <drogon/WebSocketController.h>
#include <entt/entt.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

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
    void registerNewWorker(Json::Value workerInfo) const;
};


// EnTT

struct WorkerInfo {
    boost::uuids::uuid worker_uuid;
    boost::uuids::uuid server_instance_uuid;
    Json::Value worker_info;
};

struct WebSocketConnection {
    WebSocketConnectionPtr wsConnPtr;
};

}
