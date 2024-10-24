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
                // spdlog::info("Message from Worker - {} : {}", wsConnPtr->peerAddr().toIpPort(), root.toStyledString());
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

bool verifyRegisterSecret(const Json::Value& reqJson, const std::string& register_secret)
{
    if (reqJson.isMember("register_secret") && reqJson["register_secret"].isString() && reqJson["register_secret"].asString() == register_secret) 
    {
        return true;
    }
    return false;
}



void WorkerCtrl::RegisterWorker(const std::string& workerUUID, const Json::Value& workerInfo, const WebSocketConnectionPtr& wsConnPtr) const
{   
    auto redis = drogon::app().getFastRedisClient("YLineRedis");
    // database
    auto dbClient = drogon::app().getFastDbClient("YLinedb");
    drogon::orm::Mapper<Workers> mapper(dbClient);

    mapper.findOne(
        drogon::orm::Criteria(Workers::Cols::_worker_uuid, drogon::orm::CompareOperator::EQ, workerUUID),
        [&workerUUID](const Workers &worker)
        {
            spdlog::info("Worker found in database 数据库中找到工作机: {}", workerUUID);
        },
        [&](const drogon::orm::DrogonDbException &e)
        {
            spdlog::info("Failed to find Worker in database 数据库中未找到工作机: {}", workerUUID);
            registerNewWorker(workerUUID, workerInfo, wsConnPtr);
        }
    );
    
}

void WorkerCtrl::registerNewWorker(const std::string& workerUUID, const Json::Value& workerInfo, const WebSocketConnectionPtr& wsConnPtr) const
{
    auto redis = drogon::app().getFastRedisClient("YLineRedis");
    // database
    auto dbClient = drogon::app().getFastDbClient("YLinedb");
    drogon::orm::Mapper<Workers> mapper(dbClient);

    //uuid
    const std::string& server_instance_uuid = boost::uuids::to_string(ServerSingleton::getInstance().getServerInstanceUUID());

    // 构造 worker 对象
    Workers worker;
    worker.setWorkerUuid(workerUUID);
    worker.setServerInstanceUuid(server_instance_uuid);
    worker.setWorkerEnttId(33);
     // 将 Json::Value 的 workerInfo 转换为字符串
    Json::StreamWriterBuilder writer;
    const std::string workerInfoStr = Json::writeString(writer, workerInfo);
    worker.setWorkerInfo(workerInfoStr);

    // 插入数据库 这里需要使用同步
    mapper.insert(
        worker,
        [](const Workers worker)
        {
            spdlog::info("New Worker registered into database 新工作机注册到数据库成功: {}", *worker.getWorkerUuid());
        },
        [wsConnPtr](const drogon::orm::DrogonDbException &e)
        {
            wsConnPtr->shutdown(CloseCode::kUnexpectedCondition, "Failed to register new Worker into database 注册新工作机到数据库失败");
            spdlog::error("Failed to register new Worker into database 注册新工作机到数据库失败: {}", e.base().what());
        }
    );
}


void WorkerCtrl::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr& wsConnPtr)
{
    const auto& reqPeerAddr = req->getPeerAddr();
    const auto& wsPeerAddr = wsConnPtr->peerAddr();
    spdlog::debug("{} connected to WorkerCtrl WebSocket", wsPeerAddr.toIpPort());
    auto reqJson = req->getJsonObject();
    if (reqJson) 
    {
        spdlog::info("New Worker Register Request from 新的工作机注册申请来自: {}", wsPeerAddr.toIpPort());
        spdlog::debug("Worker Request JSON: {}", reqJson->toStyledString());
        std::string register_secret = ServerSingleton::getInstance().getConfigData().register_secret;
        if (verifyRegisterSecret(*reqJson, register_secret)) 
        {
            spdlog::debug("Worker Register JSON: {}", reqJson->toStyledString());
        } 
        else 
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
        RegisterWorker((*reqJson)["worker_uuid"].asString(), (*reqJson)["worker_info"], wsConnPtr);
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
