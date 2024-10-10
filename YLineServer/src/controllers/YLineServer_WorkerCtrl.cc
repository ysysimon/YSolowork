#include "YLineServer_WorkerCtrl.h"
#include "spdlog/spdlog.h"

using namespace YLineServer;

void WorkerCtrl::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr, std::string &&message, const WebSocketMessageType &type)
{
    // write your application logic here
    wsConnPtr->send("haha");
    spdlog::info("Received message: {}", message);
    
}

void WorkerCtrl::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr& wsConnPtr)
{
    // write your application logic here
    const auto& reqPeerAddr = req->getPeerAddr();
    const auto& wsPeerAddr = wsConnPtr->peerAddr();
    spdlog::info("{} want to connect to WorkerCtrl WebSocket", reqPeerAddr.toIpPort());
    spdlog::info("{} connected to WorkerCtrl WebSocket", wsPeerAddr.toIpPort());
    wsConnPtr->send("hello");
}

void WorkerCtrl::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr)
{
    // write your application logic here
    const auto& wsPeerAddr = wsConnPtr->peerAddr();
    spdlog::info("{} disconnected from WorkerCtrl WebSocket", wsPeerAddr.toIpPort());
}
