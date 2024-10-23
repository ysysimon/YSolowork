#include "YLineServer_WorkerCtrl.h"
#include "drogon/WebSocketConnection.h"

#include "drogon/orm/Mapper.h"
#include "models/Workers.h"

#include "spdlog/spdlog.h"
#include <json/reader.h>
#include <string>

#include "utils/server.h"


using namespace YLineServer;
using namespace drogon_model::yline;

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
                spdlog::info("Message from Worker - {} : {}", wsConnPtr->peerAddr().toIpPort(), root.toStyledString());
                // // 根据 "action" 字段处理不同的指令
                // if (root.isMember("action")) {
                //     std::string action = root["action"].asString();
                //     if (action == "sendMessage") {
                //         std::string content = root["content"].asString();
                        
                //     }
                // }
            } else {
                spdlog::error("Message from Worker - {} : Failed to parse JSON message, {}", wsConnPtr->peerAddr().toIpPort(), errs);
            }
        } catch (const std::exception& e) {
            spdlog::error("Message from Worker - {} : Failed to parse JSON message, {}", wsConnPtr->peerAddr().toIpPort(), e.what());
        }
    }
    
}

bool verifyRegisterSecret(const Json::Value &reqJson, const std::string &register_secret)
{
    if (reqJson.isMember("register_secret") && reqJson["register_secret"].isString() && reqJson["register_secret"].asString() == register_secret) 
    {
        return true;
    }
    return false;
}



void WorkerCtrl::registerNewWorker(Json::Value workerInfo) const
{
    // auto redis = drogon::app().getFastRedisClient("YLineRedis");
    // database
    auto dbClient = drogon::app().getFastDbClient("YLinedb");
    drogon::orm::Mapper<Workers> mapper(dbClient);

    //uuid
    boost::uuids::random_generator uuidgen;
    boost::uuids::uuid worker_uuid = uuidgen();
    boost::uuids::uuid server_instance_uuid = ServerSingleton::getInstance().getServerInstanceUUID();


}


void WorkerCtrl::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr& wsConnPtr)
{
    const auto& reqPeerAddr = req->getPeerAddr();
    const auto& wsPeerAddr = wsConnPtr->peerAddr();
    spdlog::debug("{} connected to WorkerCtrl WebSocket", wsPeerAddr.toIpPort());
    auto reqJson = req->getJsonObject();
    if (reqJson) 
    {
        spdlog::debug("Worker Request JSON: {}", reqJson->toStyledString());
        std::string register_secret = ServerSingleton::getInstance().getConfigData().register_secret;
        if (verifyRegisterSecret(*reqJson, register_secret)) 
        {
            


            spdlog::debug("Worker Register JSON: {}", reqJson->toStyledString());
            spdlog::info("{} registered as Worker 成功注册工作机", wsPeerAddr.toIpPort());
        } 
        else 
        {
            wsConnPtr->shutdown(CloseCode::kUnexpectedCondition, "Invalid Worker Register Secret");
            spdlog::error("{} failed to register as Worker, invalid register secret 注册工作机失败, 无效的注册密钥", wsPeerAddr.toIpPort());
        }
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
    spdlog::info("{} disconnected from WorkerCtrl WebSocket", wsPeerAddr.toIpPort());
}
