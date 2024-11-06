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
            case CommandType::requireWorkerInfo:
            {
                if (!json.isMember("first") || !json.isMember("last"))
                {
                    spdlog::error("{} - No first or last in requireWorkerInfo command", wsConnPtr->peerAddr().toIpPort());
                    return;
                
                }
                if (!json["first"].isInt64() || !json["last"].isInt64()) 
                {
                    spdlog::error("{} - first or last is not int64 in requireWorkerInfo command", wsConnPtr->peerAddr().toIpPort());
                    return;
                }

                CommandsetWorkerStatus(wsConnPtr, json["first"].asInt64(), json["last"].asInt64());
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
    CommandsetWorkerCount(wsConnPtr);
}

void WorkerStatusCtrl::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr)
{
    const auto& wsPeerAddr = wsConnPtr->peerAddr();
    spdlog::debug("{} disconnected from WorkerCtrl WebSocket", wsPeerAddr.toIpPort());
}

void WorkerStatusCtrl::CommandsetWorkerCount(const WebSocketConnectionPtr& wsConnPtr)
{
    auto dbClient = drogon::app().getFastDbClient("YLinedb");
    dbClient->execSqlAsync(
        "SELECT COUNT(*) FROM workers",
        [wsConnPtr](const drogon::orm::Result &result) 
        {
            int64_t count = result[0][0].as<int64_t>();
            Json::Value json;
            json["command"] = "setWorkerCount";
            json["count"] = count;
            wsConnPtr->sendJson(json);
            spdlog::info("{} Requested Worker Status 请求工作机总数", wsConnPtr->peerAddr().toIpPort());
        },
        [wsConnPtr](const drogon::orm::DrogonDbException &err) 
        {
            // 如果查询失败，直接关闭连接，由客户端重连发起再一次查询
            wsConnPtr->shutdown(CloseCode::kUnexpectedCondition, "Failed to query Worker database 查询工作机总数失败");
            spdlog::error("Failed to query Worker database 查询工作机总数失败: {}", err.base().what());
        }
    );
}

void WorkerStatusCtrl::CommandsetWorkerStatus(const WebSocketConnectionPtr& wsConnPtr, const Json::Int64 fist, const Json::Int64 last)
{
    auto redis = drogon::app().getFastRedisClient("YLineRedis");
}