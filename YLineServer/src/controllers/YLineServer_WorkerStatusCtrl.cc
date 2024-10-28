#include "YLineServer_WorkerStatusCtrl.h"
#include "spdlog/spdlog.h"
#include "models/Workers.h"

using namespace YLineServer;
using namespace drogon_model::yline;

void WorkerStatusCtrl::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr, std::string &&message, const WebSocketMessageType &type)
{
    // write your application logic here
}

void WorkerStatusCtrl::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr& wsConnPtr)
{
    const auto& wsPeerAddr = wsConnPtr->peerAddr();
    spdlog::debug("{} connected to WorkerCtrl WebSocket", wsPeerAddr.toIpPort());

    // database
    auto dbClient = drogon::app().getFastDbClient("YLinedb");
    // drogon::orm::Mapper<Workers> mapper(dbClient);
    dbClient->execSqlAsync(
    "SELECT COUNT(*) FROM workers",
    [wsConnPtr](const drogon::orm::Result &result) 
    {
        int64_t count = result[0][0].as<int64_t>();
        spdlog::info("{} Requested Worker Status 请求工作机状态", wsConnPtr->peerAddr().toIpPort());
    },
    [wsConnPtr](const drogon::orm::DrogonDbException &err) 
    {
        wsConnPtr->shutdown(CloseCode::kUnexpectedCondition, "Failed to query Worker database 查询工作机数据库失败");
        spdlog::error("Failed to query Worker database 查询工作机数据库失败: {}", err.base().what());
    }
);
}

void WorkerStatusCtrl::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr)
{
    const auto& wsPeerAddr = wsConnPtr->peerAddr();
    spdlog::debug("{} disconnected from WorkerCtrl WebSocket", wsPeerAddr.toIpPort());
}
