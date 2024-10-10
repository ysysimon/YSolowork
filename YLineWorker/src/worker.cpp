#include "worker.h"
#include "drogon/HttpClient.h"
#include "drogon/IntranetIpFilter.h"
#include "drogon/LocalHostFilter.h"
#include <drogon/drogon.h>
#include "utils/logger.h"
#include "utils/api.h"

#include <format>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <string>
#include <trantor/utils/Logger.h>


namespace YLineWorker {

void spawnWorker(const Config& config, const std::shared_ptr<spdlog::logger> custom_logger)
{
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
    // auto resp404 = drogon::HttpResponse::newNotFoundResponse();
    // resp404->setStatusCode(drogon::k404NotFound);
    // resp404->setContentTypeCode(drogon::CT_TEXT_HTML);
    // resp404->setBody("404 Not Found, YLineServer 不存在该端点");
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
    spdlog::info("Connecting to server 连接到服务器: {}", conn_str);
    auto client = drogon::WebSocketClient::newWebSocketClient(
        conn_str
    );
    // 设置 worker 数据
    WorkerSingleton::getInstance().setWorkerData({
        .worker_id = "worker_id",
        .worker_name = "worker_name",
        .client = client
    });
    // 连接到服务器
    WorkerSingleton::getInstance().connectToServer();
    

    spdlog::info("YLineWorker Service started 服务已启动");
    drogon::app().run();

}

// 单例类: Worker 定义
WorkerSingleton& WorkerSingleton::getInstance() {
    static WorkerSingleton instance;  // 静态局部变量，确保只初始化一次
    return instance;
}

const worker& WorkerSingleton::getWorkerData() const {
    return workerData_;
}

void WorkerSingleton::setWorkerData(const worker& workerData) {
    workerData_ = workerData;
}

void WSconnectCallback(ReqResult result, const HttpResponsePtr& resp, const WebSocketClientPtr& wsClient) {
    if (result == ReqResult::Ok) {
        spdlog::info("Connected to server 成功连接到服务器");
    } else {
        spdlog::error("Failed to connect to server 连接服务器失败: {}", to_string(result));
    }

}

void WorkerSingleton::connectToServer() {
    // 获取 client
    auto client = getWorkerData().client;
    // 连接到服务器
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/ws/worker");
    client->connectToServer(
        req,
        WSconnectCallback
    );
}

} // namespace YLineWorker