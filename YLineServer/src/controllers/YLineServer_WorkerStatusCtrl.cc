#include "YLineServer_WorkerStatusCtrl.h"
#include "drogon/nosql/RedisResult.h"
#include "spdlog/spdlog.h"
#include "utils/server.h"
#include "utils/api.h"
#include <json/value.h>
#include <format>
#include "models/Workers.h"

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
            case CommandType::requireWorkers:
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

                CommandrequireWorkerInfo(wsConnPtr, json["first"].asInt64(), json["last"].asInt64());
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
            json["data"] = count;
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

void WorkerStatusCtrl::CommandrequireWorkerInfo(const WebSocketConnectionPtr& wsConnPtr, const Json::Int64 fist, const Json::Int64 last)
{
    // all right maybe let's not optimize too early
    // worker list is not that big maybe we can just query the database

    // auto redis = drogon::app().getFastRedisClient("YLineRedis");
    // redis->execCommandAsync(
    //     [wsConnPtr, fist, last](const drogon::nosql::RedisResult &r) 
    //     {
    //         if(r.type() == drogon::nosql::RedisResultType::kArray)
    //         {
    //             const auto& workers = r.asArray();
    //             if (workers.size() != (last - fist + 1) || workers.empty())
    //             {
    //                 spdlog::info(
    //                     "{} - WorkerList size mismatch trigger synchronization 工作机列表不匹配, 触发同步: {} != {}", 
    //                     wsConnPtr->peerAddr().toIpPort(), workers.size(), last - fist + 1
    //                 );
    //                 // database
    //                 auto dbClient = drogon::app().getFastDbClient("YLinedb");
    //                 drogon::orm::Mapper<Workers> mapper(dbClient);
    //                 mapper.orderBy(Workers::Cols::_id, drogon::orm::SortOrder::ASC)
    //                         .limit(last - fist + 1)
    //                         .offset(fist)
    //                         .findAll(
    //                             [wsConnPtr](const std::vector<Workers>& workers)
    //                             {
    //                                 auto redis = drogon::app().getFastRedisClient("YLineRedis");
    //                                 Json::Value json;
    //                                 json["command"] = "setWorkerInfo";
    //                                 for(const auto& worker : workers)
    //                                 {
    //                                     json["workers"].append(worker.toJson());
    //                                 }
    //                                 wsConnPtr->sendJson(json);
    //                                 spdlog::info("{} Requested Worker Info 请求工作机信息", wsConnPtr->peerAddr().toIpPort());
    //                             },
    //                             [wsConnPtr](const drogon::orm::DrogonDbException &err)
    //                             {
    //                                 wsConnPtr->shutdown(CloseCode::kUnexpectedCondition, "Failed to query Worker database 查询工作机信息失败");
    //                                 spdlog::error("{} - Failed to query Worker database 查询工作机信息失败: {}", wsConnPtr->peerAddr().toIpPort(), err.base().what());
    //                             }
    //                         );

    //                 return;
    //             }

    //             Json::Value json;
    //             json["command"] = "setWorkerInfo";
    //             for(const auto& worker : r.asArray())
    //             {
    //                 json["workers"].append(worker.asString());
    //             }
    //             wsConnPtr->sendJson(json);
    //             spdlog::info("{} Requested Worker Info 请求工作机信息", wsConnPtr->peerAddr().toIpPort());
    //         }
    //         else
    //         {
    //             wsConnPtr->shutdown(CloseCode::kUnexpectedCondition, "Unexpected Redis result type");
    //             spdlog::error("{} - Unexpected Redis result type: {}", wsConnPtr->peerAddr().toIpPort(), static_cast<int>(r.type()));
    //         }
    //     },
    //     [wsConnPtr](const std::exception &err)
    //     {
    //         wsConnPtr->shutdown(CloseCode::kUnexpectedCondition, "Failed to query Worker Redis 查询工作机信息失败");
    //         spdlog::error("{} - Failed to query Worker database 查询工作机信息失败: {}", wsConnPtr->peerAddr().toIpPort(), err.what());
    //     },
    //     std::format("ZRANGE WorkerList {} {}", fist, last).c_str()
    // );



    // database
    auto dbClient = drogon::app().getFastDbClient("YLinedb");
    drogon::orm::Mapper<Workers> mapper(dbClient);
    mapper.orderBy(Workers::Cols::_id, drogon::orm::SortOrder::ASC)
            .limit(last - fist + 1)
            .offset(fist)
            .findAll(
                [wsConnPtr, fist](const std::vector<Workers>& workers)
                {
                    auto redis = drogon::app().getFastRedisClient("YLineRedis");
                    Json::Value json;
                    json["command"] = "setWorkers";
                    json["data"]["first"] = fist;
                    for(const auto& worker : workers)
                    {
                        json["data"]["workers"].append(worker.toJson());
                    }
                    wsConnPtr->sendJson(json);
                    spdlog::info("{} Requested Worker Info 请求工作机信息", wsConnPtr->peerAddr().toIpPort());
                },
                [wsConnPtr](const drogon::orm::DrogonDbException &err)
                {
                    wsConnPtr->shutdown(CloseCode::kUnexpectedCondition, "Failed to query Worker database 查询工作机信息失败");
                    spdlog::error("{} - Failed to query Worker database 查询工作机信息失败: {}", wsConnPtr->peerAddr().toIpPort(), err.base().what());
                }
            );
}