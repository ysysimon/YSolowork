#include "YLineServer_JobStatusCtrl.h"

#include "json/value.h"
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

                CommandrequireJobInfo(wsConnPtr, json["first"].asInt64(), json["last"].asInt64());
                break;
            }
            case CommandType::requireJobStatus:
            {
                if (!json.isMember("job_id"))
                {
                    spdlog::error("{} - No job_id in requireJobStatus command", wsConnPtr->peerAddr().toIpPort());
                    return;
                }

                CommandrequireJobStatus(wsConnPtr, json["job_id"].asInt64());
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
    auto dbClient = drogon::app().getFastDbClient("YLinedb");
    drogon::orm::Mapper<Jobs> mapper(dbClient);
    mapper.orderBy(Jobs::Cols::_id, drogon::orm::SortOrder::ASC)
            .limit(last - fist + 1)
            .offset(fist)
            .findAll(
                [wsConnPtr, fist](const std::vector<Jobs> &jobs)
                {
                    Json::Value json;
                    json["command"] = "setJobs";
                    json["data"]["first"] = fist;
                    json["data"]["last"] = fist + jobs.size();
                    for(const auto& job : jobs)
                    {
                        json["data"]["jobs"].append(job.toJson());
                    }
                    wsConnPtr->sendJson(json);
                    spdlog::info("{} Requested Job Info 请求工作信息", wsConnPtr->peerAddr().toIpPort());
                },
                [wsConnPtr](const drogon::orm::DrogonDbException &err)
                {
                    wsConnPtr->shutdown(CloseCode::kUnexpectedCondition, "Failed to query Job database 查询工作信息失败");
                    spdlog::error("{} - Failed to query Job database 查询工作信息失败: {}", wsConnPtr->peerAddr().toIpPort(), err.base().what());
                }
            );
}

void
JobStatusCtrl::CommandrequireJobStatus(const WebSocketConnectionPtr& wsConnPtr, const Json::Int64 job_id)
{
    spdlog::debug("{} Requested Job {} Status 请求工作状态", wsConnPtr->peerAddr().toIpPort(), job_id);
    auto redis = drogon::app().getFastRedisClient("YLineRedis");
    redis->execCommandAsync(
        [wsConnPtr, job_id](const drogon::nosql::RedisResult &r)
        {
            Json::Value json;
            json["command"] = "setJobStatus";
            json["data"]["job_id"] = job_id;

            // todo: send job status
        },
        [wsConnPtr, job_id](const std::exception &err)
        {
            // todo: send error message
        },
        std::format("").c_str()
    );
}