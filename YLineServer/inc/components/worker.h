#ifndef YLineServer_Componets_WORKER_H
#define YLineServer_Componets_WORKER_H

#include <boost/uuid/uuid.hpp>
#include "drogon/WebSocketConnection.h"

#include <amqpcpp/include/amqpcpp.h>

#include <entt/signal/sigh.hpp>

namespace YLineServer::Components
{

struct Worker {
    boost::uuids::uuid worker_uuid;
    boost::uuids::uuid server_instance_uuid;
    Json::Value worker_info;
    drogon::WebSocketConnectionPtr wsConnPtr;
};

}; // namespace YLineServer::Components



#endif // YLineServer_Componets_WORKER_H