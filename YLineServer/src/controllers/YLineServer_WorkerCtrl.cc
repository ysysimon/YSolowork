#include "YLineServer_WorkerCtrl.h"
#include "drogon/WebSocketConnection.h"

#include "spdlog/spdlog.h"
#include "json/writer.h"
#include <format>
#include <json/reader.h>
#include <string>
#include <utility>
#include <vector>

#include "utils/server.h"


using namespace YLineServer;

void WorkerCtrl::writeUsage2redis(const Json::Value& usageJson, const WebSocketConnectionPtr& wsConnPtr) const
{
    auto redis = drogon::app().getFastRedisClient("YLineRedis");
    const auto& workerUUID = ServerSingleton::getInstance().wsConnToWorkerUUID[wsConnPtr];
    std::string workerUUIDStr = boost::uuids::to_string(workerUUID);
    // std::vector<std::string> redisHashfileds;
    std::vector<std::pair<std::string, std::string>> redisHashfileds;

    // redis hash 设置指令, 用于设置工作机的使用情况
    redisHashfileds.emplace_back("workerIP", wsConnPtr->peerAddr().toIp());
    if (usageJson.isMember("cpuUsage")) 
    {
        redisHashfileds.emplace_back("cpuUsage", usageJson["cpuUsage"].asString());
    }
    else 
    {
        redisHashfileds.emplace_back("cpuUsage", "-1");
    }

    if (usageJson.isMember("cpuMemoryUsage")) 
    {
        redisHashfileds.emplace_back("cpuMemoryUsage", usageJson["cpuMemoryUsage"].asString());
    }
    else 
    {
        redisHashfileds.emplace_back("cpuMemoryUsage", "-1");
    }
    std::string redisCommand = std::format(
        "HMSET WorkerUsage:{}", 
        workerUUIDStr
    );
    for(const auto& [field, value] : redisHashfileds) 
    {
        redisCommand += std::format(" {} {}", field, value);
    }
    // 设置 WorkerUsage hash
    redis->execCommandAsync
    (
        [](const drogon::nosql::RedisResult &r) 
        {
            // spdlog::debug("HMSET WorkerUsage result: {}", r.asString());
        },
        [wsConnPtr](const std::exception &err)
        {
            spdlog::error("{} - HMSET WorkerUsage error: {}", wsConnPtr->peerAddr().toIpPort(), err.what());
        },
        redisCommand.c_str()
    );
    // 设置 WorkerUsage hash 的过期时间
    redis->execCommandAsync
    (
        [](const drogon::nosql::RedisResult &r) 
        {
            //spdlog::debug("EXPIRE WorkerUsage result: {}", r.asString());
        },
        [wsConnPtr](const std::exception &err)
        {
            spdlog::error("{} - EXPIRE WorkerUsage error: {}", wsConnPtr->peerAddr().toIpPort(), err.what());
        },
        std::format("EXPIRE WorkerUsage:{} 3", workerUUIDStr).c_str()
    );

    // 如果有 gpuUsage 字段, 则设置 gpuUsage 表
    if (usageJson.isMember("gpuUsage")) 
    {
        // Nvidia
        if (usageJson["gpuUsage"].isMember("NVIDIA")) 
        {

            const auto& nvidiaUsages = usageJson["gpuUsage"]["NVIDIA"];
            Json::FastWriter fastWriter;
            std::string dumpedNvidiaUsages = fastWriter.write(nvidiaUsages);
            // spdlog::debug("NVIDIA GPU Usage: {}", dumpedNvidiaUsages);
            // 设置 WorkerUsage:NVIDIA hash 同时设置过期时间
            redis->execCommandAsync
            (
                [](const drogon::nosql::RedisResult &r) 
                {
                    // spdlog::debug("SETEX WorkerUsage:NVIDIA result: {}", r.asString());
                },
                [wsConnPtr](const std::exception &err)
                {
                    spdlog::error("{} - SETEX WorkerUsage:NVIDIA error: {}", wsConnPtr->peerAddr().toIpPort(), err.what());
                },
                std::format("SETEX WorkerUsage:NVIDIA:{} 3 {}", workerUUIDStr, dumpedNvidiaUsages).c_str()
            );
        }
    }

    // don't know why redis->newTransactionAsync does not work
    // redis->newTransactionAsync
    // (
    //     [redisCommand, workerUUIDStr](const drogon::nosql::RedisTransactionPtr& transPtr) 
    //     {
    //         if (transPtr == nullptr) 
    //         {
    //             spdlog::error("Failed to create usage Redis transaction");
    //             return;
    //         }
    //         // 设置 WorkerUsage hash
    //         transPtr->execCommandAsync
    //         (
    //             [](const drogon::nosql::RedisResult &r) 
    //             {
    //                 spdlog::debug("HMSET WorkerUsage result: {}", r.asString());
    //             },
    //             [](const std::exception &err)
    //             {
    //                 spdlog::error("HMSET WorkerUsage error: {}", err.what());
    //             },
    //             redisCommand.c_str()
    //         );
    //         // 设置 WorkerUsage hash 的过期时间
    //         transPtr->execCommandAsync
    //         (
    //             [](const drogon::nosql::RedisResult &r) 
    //             {
    //                 spdlog::debug("EXPIRE WorkerUsage result: {}", r.asString());
    //             },
    //             [](const std::exception &err)
    //             {
    //                 spdlog::error("EXPIRE WorkerUsage error: {}", err.what());
    //             },
    //             std::format("EXPIRE WorkerUsage:{} 3", workerUUIDStr).c_str()
    //         );
    //         // 执行事务
    //         transPtr->execute
    //         (
    //             [](const drogon::nosql::RedisResult &r) 
    //             {
    //                 spdlog::debug("Transaction WorkerUsage result: {}", r.asString());
    //             },
    //             [](const std::exception &err)
    //             {
    //                 spdlog::error("Transaction WorkerUsage error: {}", err.what());
    //             }
    //         );
    //     }
    // );

}

void WorkerCtrl::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr, std::string &&message, const WebSocketMessageType &type)
{
    if (type == WebSocketMessageType::Text)
    {
        try {
            Json::Value root;
            Json::CharReaderBuilder reader;
            std::string errs;
            std::istringstream s(message);
            if (Json::parseFromStream(reader, s, &root, &errs)) {
                // spdlog::info("Message from Worker - {} : {}", wsConnPtr->peerAddr().toIpPort(), root.toStyledString());
                // 根据 "command" 字段处理不同的指令
                if (root.isMember("command")) 
                {
                    CommandType command;
                    if (commandMap.find(root["command"].asString()) == commandMap.end()) 
                    {
                        command = CommandType::UNKNOWN;
                    }
                    else 
                    {
                        command = commandMap[root["command"].asString()];
                    }
                    switch (command) 
                    {
                        case CommandType::usage:
                            spdlog::trace("Message from Worker - {} : Command: usage", wsConnPtr->peerAddr().toIpPort());
                            // spdlog::debug("Message from Worker - {} : Usage JSON: {}", wsConnPtr->peerAddr().toIpPort(), root.toStyledString());
                            writeUsage2redis(root, wsConnPtr);
                            break;
                        case CommandType::UNKNOWN:
                            spdlog::warn("Message from Worker - {} : Unknown Command: {}", wsConnPtr->peerAddr().toIpPort(), root["command"].asString());
                            break;
                    }
                    
                }
                else 
                {
                    spdlog::error("Message from Worker - {} : Invalid JSON message, no command field", wsConnPtr->peerAddr().toIpPort());
                }
            } else {
                spdlog::error("Message from Worker - {} : Failed to parse JSON message, {}", wsConnPtr->peerAddr().toIpPort(), errs);
            }
        } catch (const std::exception& e) {
            spdlog::error("Message from Worker - {} : Failed to parse JSON message, {}", wsConnPtr->peerAddr().toIpPort(), e.what());
        }
    }
    
}

bool verifyRegisterSecret(const Json::Value& reqJson, const std::string& register_secret)
{
    if (reqJson.isMember("register_secret") && reqJson["register_secret"].isString() && reqJson["register_secret"].asString() == register_secret) 
    {
        return true;
    }
    return false;
}

void WorkerCtrl::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr& wsConnPtr)
{
    // const auto& reqPeerAddr = req->getPeerAddr();
    const auto& wsPeerAddr = wsConnPtr->peerAddr();
    spdlog::debug("{} connected to WorkerCtrl WebSocket", wsPeerAddr.toIpPort());
    auto reqJson = req->getJsonObject();
    if (reqJson) 
    {
        spdlog::info("New Worker Register Request from 新的工作机注册申请来自: {}", wsPeerAddr.toIpPort());
        spdlog::debug("Worker Request JSON: {}", reqJson->toStyledString());
        std::string register_secret = ServerSingleton::getInstance().getConfigData().register_secret;
        if (!verifyRegisterSecret(*reqJson, register_secret)) 
        {
            wsConnPtr->shutdown(CloseCode::kUnexpectedCondition, "Invalid Worker Register Secret");
            spdlog::error("{} failed to register as Worker, invalid register secret 注册工作机失败, 无效的注册密钥", wsPeerAddr.toIpPort());
            return;
        } 

        if (!(*reqJson).isMember("worker_uuid") && !(*reqJson)["worker_uuid"].isString()) 
        {
            wsConnPtr->shutdown(CloseCode::kInvalidMessage, "Invalid Worker Register UUID");
            spdlog::error("{} failed to register as Worker, invalid UUID 注册工作机, 无效的UUID", wsPeerAddr.toIpPort());
            return;
        }

        if(!(*reqJson).isMember("worker_info") && !(*reqJson)["worker_info"].isObject())
        {
            wsConnPtr->shutdown(CloseCode::kInvalidMessage, "Invalid Worker Register Info");
            spdlog::error("{} failed to register as Worker, invalid Worker Info 注册工作机, 无效的工作机信息", wsPeerAddr.toIpPort());
            return;
        }

        // 回调地狱，但是没辙 wsCtrl 不支持协程写法
        // callback hell but no choice, wsCtrl does not support coroutine
        registerWorker((*reqJson)["worker_uuid"].asString(), (*reqJson)["worker_info"], wsConnPtr);
    }

    if (!req->getJsonError().empty()) 
    {
        wsConnPtr->shutdown(CloseCode::kInvalidMessage, "Invalid Worker Register JSON");
        spdlog::error("Request JSON error: {}", req->getJsonError());
    }

}

void WorkerCtrl::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr)
{
    const auto& wsPeerAddr = wsConnPtr->peerAddr();

    {

    // 从 hash 表 和 EnTT 注册表中删除工作机
    std::unique_lock<std::shared_mutex> lock(ServerSingleton::getInstance().workerUUIDMapMutex);
    std::unique_lock<std::shared_mutex> lock2(ServerSingleton::getInstance().wsConnMapMutex);
    auto it = ServerSingleton::getInstance().wsConnToWorkerUUID.find(wsConnPtr);
    if (it != ServerSingleton::getInstance().wsConnToWorkerUUID.end())
    {
        EnTTidType workerEnTTid = ServerSingleton::getInstance().WorkerUUIDtoEnTTid[it->second];
        ServerSingleton::getInstance().Registry.destroy(workerEnTTid);
        ServerSingleton::getInstance().WorkerUUIDtoEnTTid.erase(it->second);
        ServerSingleton::getInstance().wsConnToWorkerUUID.erase(it);
    }

    } // end of lock scope

    spdlog::debug("{} disconnected from WorkerCtrl WebSocket", wsPeerAddr.toIpPort());
}
