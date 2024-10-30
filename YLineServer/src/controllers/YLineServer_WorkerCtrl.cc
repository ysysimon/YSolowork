#include "YLineServer_WorkerCtrl.h"
#include "drogon/WebSocketConnection.h"

#include "spdlog/spdlog.h"
#include <json/reader.h>
#include <string>

#include "utils/server.h"


using namespace YLineServer;

void WorkerCtrl::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr, std::string &&message, const WebSocketMessageType &type)
{
    if (type == WebSocketMessageType::Text)
    {
        auto redis = drogon::app().getFastRedisClient("YLineRedis");
        try {
            Json::Value root;
            Json::CharReaderBuilder reader;
            std::string errs;
            std::istringstream s(message);
            if (Json::parseFromStream(reader, s, &root, &errs)) {
                // spdlog::info("Message from Worker - {} : {}", wsConnPtr->peerAddr().toIpPort(), root.toStyledString());
                // 根据 "command" 字段处理不同的指令
                if (root.isMember("command")) {
                    CommandType command = commandMap[root["command"].asString()];
                    switch (command) 
                    {
                        case CommandType::usage:
                            spdlog::info("Message from Worker - {} : Command: usage", wsConnPtr->peerAddr().toIpPort());
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
