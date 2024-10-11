#include "YLineServer_WorkerCtrl.h"
#include "spdlog/spdlog.h"
#include <json/reader.h>

using namespace YLineServer;

void WorkerCtrl::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr, std::string &&message, const WebSocketMessageType &type)
{
    // write your application logic here
    if (type == WebSocketMessageType::Text)
    {
        Json::Value json;
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

void WorkerCtrl::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr& wsConnPtr)
{
    // write your application logic here
    const auto& reqPeerAddr = req->getPeerAddr();
    const auto& wsPeerAddr = wsConnPtr->peerAddr();
    spdlog::info("{} want to connect to WorkerCtrl WebSocket", reqPeerAddr.toIpPort());
    spdlog::info("{} connected to WorkerCtrl WebSocket", wsPeerAddr.toIpPort());
}

void WorkerCtrl::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr)
{
    // write your application logic here
    const auto& wsPeerAddr = wsConnPtr->peerAddr();
    spdlog::info("{} disconnected from WorkerCtrl WebSocket", wsPeerAddr.toIpPort());
}
