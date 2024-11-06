#include "YLineServer_WorkerStatusCtrl.h"
#include "spdlog/spdlog.h"
#include "utils/server.h"
#include "utils/api.h"
#include <json/value.h>
#include <memory>

using namespace YLineServer;

void WorkerStatusCtrl::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr, std::string &&message, const WebSocketMessageType &type)
{
    // do authentication
    if(type == WebSocketMessageType::Text)
    {
        Json::Value json;
        std::string errs;
        if(!Api::parseJson(message, json, errs))
        {
            spdlog::error("{} - Failed to parse JSON message: {}", wsConnPtr->peerAddr().toIpPort(), errs);
            return;
        }

        if (!json.isMember("command"))
        {
            spdlog::error("{} - No command in JSON message", wsConnPtr->peerAddr().toIpPort());
            return;
        }

        CommandType command;
        if (commandMap.find(json["command"].asString()) == commandMap.end())
        {
            command = CommandType::UNKNOWN;
        }
        else
        {
            command = commandMap[json["command"].asString()];
        }

        switch (command)
        {
            case CommandType::auth:
            {
                if (!json.isMember("token"))
                {
                    spdlog::error("{} - No token in auth command", wsConnPtr->peerAddr().toIpPort());
                    return;
                }

                const std::string& token = json["token"].asString();
                Api::authWebSocketConnection(wsConnPtr, token);
                break;
            }
            
            default:
                spdlog::error("{} - Unrecognized command: {}", wsConnPtr->peerAddr().toIpPort(), json["command"].asString());
                break;
        }
    }

    // get context to check if user is authenticated
    const auto& user = wsConnPtr->getContext<EnTTidType>();
    if (!user) 
    {
        Json::Value json;
        json["command"] = "requireAuth";
        wsConnPtr->sendJson(json);
        // wsConnPtr->shutdown(CloseCode::kViolation, "Unauthorized");
        return;
    }
    
}

void WorkerStatusCtrl::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr& wsConnPtr)
{
    const auto& wsPeerAddr = wsConnPtr->peerAddr();
    spdlog::debug("{} connected to WorkerCtrl WebSocket", wsPeerAddr.toIpPort());
    // database
    auto dbClient = drogon::app().getFastDbClient("YLinedb");
    // drogon::orm::Mapper<Workers> mapper(dbClient);
    dbClient->execSqlAsync(
        "SELECT COUNT(*) FROM workers",
        [wsConnPtr](const drogon::orm::Result &result) 
        {
            int64_t count = result[0][0].as<int64_t>();
            spdlog::info("{} Requested Worker Status 请求工作机状态", wsConnPtr->peerAddr().toIpPort());
        },
        [wsConnPtr](const drogon::orm::DrogonDbException &err) 
        {
            wsConnPtr->shutdown(CloseCode::kUnexpectedCondition, "Failed to query Worker database 查询工作机数据库失败");
            spdlog::error("Failed to query Worker database 查询工作机数据库失败: {}", err.base().what());
        }
    );
}

void WorkerStatusCtrl::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr)
{
    const auto& wsPeerAddr = wsConnPtr->peerAddr();
    spdlog::debug("{} disconnected from WorkerCtrl WebSocket", wsPeerAddr.toIpPort());
}
