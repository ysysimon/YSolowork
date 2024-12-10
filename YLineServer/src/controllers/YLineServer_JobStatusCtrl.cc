#include "YLineServer_JobStatusCtrl.h"

#include <spdlog/spdlog.h>
#include "utils/api.h"
#include "utils/server.h"
#include "models/Jobs.h"

using namespace YLineServer;

void JobStatusCtrl::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr, std::string &&message, const WebSocketMessageType &type)
{
    // do authentication
    if (type == WebSocketMessageType::Text) 
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

        // auth command is special
        if (command == CommandType::auth)
        {
            if (!json.isMember("token"))
            {
                spdlog::error("{} - No token in auth command", wsConnPtr->peerAddr().toIpPort());
                Json::Value json;
                json["command"] = "requireAuth";
                wsConnPtr->sendJson(json);
                wsConnPtr->shutdown(CloseCode::kViolation, "Unauthorized");
                return;
            }
            const std::string& token = json["token"].asString();
            Api::authWebSocketConnection(wsConnPtr, token, "JobStatusCtrl");
            return;
        }

        // get context to check if user is authenticated
        const auto& user = wsConnPtr->getContext<EnTTidType>();
        if (!user) 
        {
            Json::Value json;
            json["command"] = "requireAuth";
            wsConnPtr->sendJson(json);
            wsConnPtr->shutdown(CloseCode::kViolation, "Unauthorized");
            return;
        }

        // other commands
        switch (command)
        {
            case CommandType::requireJobs:
            {
                if (!json.isMember("first") || !json.isMember("last"))
                {
                    spdlog::error("{} - No first or last in requireJobsInfo command", wsConnPtr->peerAddr().toIpPort());
                    return;
                
                }
                if (!json["first"].isInt64() || !json["last"].isInt64()) 
                {
                    spdlog::error("{} - first or last is not int64 in requireJobsInfo command", wsConnPtr->peerAddr().toIpPort());
                    return;
                }

                // CommandrequireWorkerInfo(wsConnPtr, json["first"].asInt64(), json["last"].asInt64());
                break;
            }
            case CommandType::requireJobStatus:
            {
                if (!json.isMember("workerUUID"))
                {
                    spdlog::error("{} - No workerUUID in requireJobStatus command", wsConnPtr->peerAddr().toIpPort());
                    return;
                }

                // CommandrequireWorkerStatus(wsConnPtr, json["workerUUID"].asString());
                break;
            }
        
            default:
                spdlog::error("{} - Unrecognized command: {}", wsConnPtr->peerAddr().toIpPort(), json["command"].asString());
                break;
        }
    }
}

void JobStatusCtrl::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr& wsConnPtr)
{
    const auto &wsPeerAddr = wsConnPtr->peerAddr();
    spdlog::debug("{} connected to JobCtrl WebSocket", wsPeerAddr.toIpPort());
    CommandsetJobCount(wsConnPtr);
}

void JobStatusCtrl::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr)
{
    const auto &wsPeerAddr = wsConnPtr->peerAddr();
    spdlog::debug("{} disconnected from JobCtrl WebSocket", wsPeerAddr.toIpPort());
}

void
JobStatusCtrl::CommandsetJobCount(const WebSocketConnectionPtr& wsConnPtr)
{
    auto dbClient = drogon::app().getFastDbClient("YLinedb");
    dbClient->execSqlAsync(
        "SELECT COUNT(*) FROM jobs",
        [wsConnPtr](const drogon::orm::Result &result)
        {
            int64_t count = result[0][0].as<int64_t>();
            Json::Value json;
            json["command"] = "setJobCount";
            json["data"] = count;
            wsConnPtr->sendJson(json);
            spdlog::info("{} Requested Job Count 请求工作总数", wsConnPtr->peerAddr().toIpPort());
        },
        [wsConnPtr](const drogon::orm::DrogonDbException &err)
        {
            // 如果查询失败，直接关闭连接，由客户端重连发起再一次查询
            wsConnPtr->shutdown(CloseCode::kUnexpectedCondition, "Failed to query Job database 查询工作总数失败");
            spdlog::error("Failed to query Job database 查询工作总数失败: {}", err.base().what());
        }
    );
}

void
JobStatusCtrl::CommandrequireJobInfo(const WebSocketConnectionPtr& wsConnPtr, const Json::Int64 fist, const Json::Int64 last)
{

}