#include "worker.h"
#include "drogon/IntranetIpFilter.h"
#include "drogon/LocalHostFilter.h"
#include "json/value.h"
#include <drogon/drogon.h>
#include "utils/logger.h"
#include "utils/api.h"

#include <format>
#include <functional>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <string>
#include <trantor/utils/Logger.h>
#include <magic_enum.hpp>



namespace YLineWorker {

void spawnWorker(const Config& config, const std::shared_ptr<spdlog::logger> custom_logger)
{
    // 获取机器信息
    const auto& machineInfo = getMachineInfo();
    // 打印机器信息
    logWorkerMachineInfo(machineInfo);

    // 检查是否支持 spdlog
    // 注意: 需要使用 开启了 spdlog 支持的 Drogon 构建
    if (!trantor::Logger::hasSpdLogSupport()) {
        spdlog::error("Current Drogon build does not support spdlog. 当前 Drogon 构建不支持 spdlog.");
        // throw exception
        // throw std::runtime_error("Current Drogon build does not support spdlog. 当前 Drogon 构建不支持 spdlog.");
    }

    // set drogon logger
    trantor::Logger::enableSpdLog(custom_logger);
    trantor::Logger::setLogLevel(YLineWorker::spdlogToDrogonLogLevel.at(config.log_level));

    drogon::app().addListener(config.YLineWorker_ip, config.YLineWorker_port);

    // 设置自定义 404 页面
    auto resp404 = YLineWorker::Api::makeHttpResponse(
        "404 Not Found, YLineWorker 不存在该端点", 
        drogon::HttpStatusCode::k404NotFound, 
        nullptr, 
        drogon::CT_TEXT_HTML);
    drogon::app().setCustom404Page(resp404);

    // 启用 Brotli 和 Gzip 压缩
    drogon::app().enableBrotli(true).enableGzip(true);

    // 内置 中间件 middleware
    // 仅允许内网 IP 访问
    if (config.intranet_ip_filter)
    {
        // drogon::app().registerFilter<IntranetIpFilter>(); ;
        spdlog::info("Intranet IP filter enabled 内网 IP 过滤器已启用");
    }
    // 仅允许本机访问
    if (config.local_host_filter)
    {
        // drogon::app().registerMiddleware(std::make_shared<drogon::LocalHostFilter>());
        spdlog::info("Local host filter enabled 本地主机过滤器已启用");
    }

    std::string conn_str = std::format("ws://{}:{}", config.YLineServer_ip, config.YLineServer_port);
    auto client = drogon::WebSocketClient::newWebSocketClient(
        conn_str
    );
    // 设置 worker 数据
    WorkerSingleton::getInstance().setWorkerData({
        .worker_id = "wait_for_register",
        .worker_name = "wait_for_register",
        .client = client,
        .worker_machineInfo = machineInfo
    });

    // 初始化 nvml
    try {
        WorkerSingleton::getInstance().initNvml();
        spdlog::info("NVML initialized 初始化成功 NVML");
        WorkerSingleton::getInstance().logNvmlInfo();
        
    } catch (const DynamicLibraryException& e) {
        spdlog::warn("Failed to initialize NVML: {}", e.what());
        spdlog::warn("NVML related features may not work properly NVML 相关功能无法正常工作");
        spdlog::warn("This maynot be a problem if you do not have Nvidia GPU 如果您没有 Nvidia GPU, 这可能不是问题");
    }

    

    // 连接到服务器
    spdlog::info("Connecting to server 连接到服务器: {}", conn_str);
    WorkerSingleton::getInstance().connectToServer();
    

    spdlog::info("YLineWorker Service started 服务已启动");
    drogon::app().run();

}

// WebSocket 回调函数

void WSconnectCallback(ReqResult result, const HttpResponsePtr& resp, const WebSocketClientPtr& wsClient) {

    if (result == ReqResult::Ok) {
        spdlog::info("Connected to server 成功连接到服务器");
        // 获取当前的 event loop
        auto loop = trantor::EventLoop::getEventLoopOfCurrentThread();
        if (!loop) {
            spdlog::error("Failed to get current event loop 获取当前事件循环失败");
            return;
        }

        // 每秒 发送一次 使用率
        auto _usageInfotimer = loop->runEvery(1.0, [wsClient]() {
            const Json::Value& json = WorkerSingleton::getInstance().getUsageJson();
            
            if (wsClient && wsClient->getConnection() && wsClient->getConnection()->connected())
            {
                wsClient->getConnection()->sendJson(json);
            }
        });

        WorkerSingleton::getInstance().usageInfotimer = _usageInfotimer;

    } else {
        spdlog::error("Failed to connect to server 连接服务器失败: {}", to_string(result));
    }

}


Task<> msgAsyncCallback(std::string&& message,
                       const WebSocketClientPtr& client,
                       const WebSocketMessageType& type)
{
    if (type == WebSocketMessageType::Text)
    {
        // 打印接收到的消息
        spdlog::info("Received message: {}", message);
    }
    

    // 可以执行其他异步任务，比如数据库查询、网络请求等
    // co_await asyncDatabaseQuery();
    
    co_return;
}

void WSconnectClosedCallback(const WebSocketClientPtr& wsClient) {
    spdlog::warn("Server Connection closed 服务器连接已断开");
    // 取消定时器
    auto loop = trantor::EventLoop::getEventLoopOfCurrentThread();
    if (!loop) {
        spdlog::error("Failed to get current event loop 获取当前事件循环失败");
        return;
    }
    loop->invalidateTimer(WorkerSingleton::getInstance().usageInfotimer);
}

void WorkerSingleton::connectToServer() {
    // 获取 client
    const auto& client = WorkerSingleton::getInstance().workerData_.client;
    // 连接到服务器
    Json::Value json = WorkerSingleton::getInstance().getRegisterJson();
    auto req = drogon::HttpRequest::newHttpJsonRequest(json);
    req->setPath("/ws/worker");
    
    client->setAsyncMessageHandler(msgAsyncCallback);
    client->setConnectionClosedHandler(WSconnectClosedCallback);
    client->connectToServer(
        req,
        WSconnectCallback
    );
}

} // namespace YLineWorker