#include "worker.h"
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
#include <magic_enum.hpp>

#include "UTusage.h"

namespace YLineWorker {

MachineInfo getMachineInfo()
{
    MachineInfo machineInfo;
    try {
        machineInfo.systomInfo = YSolowork::untility::getSystomInfo();
    } catch (const std::exception& e) {
        spdlog::error("Failed to get system info 获取系统信息失败: {}", e.what());
        spdlog::warn("System information related features may not work properly 系统信息相关功能可能无法正常工作");
    }

    try {
        machineInfo.machineName = YSolowork::untility::getMachineName();
    } catch (const std::exception& e) {
        spdlog::error("Failed to get machine name 获取机器名失败: {}", e.what());
        spdlog::warn("Machine name related features may not work properly 机器名相关功能可能无法正常工作");
    }

    try {
        machineInfo.devices = YSolowork::untility::getAllDevices();
    } catch (const std::exception& e) {
        machineInfo.devices.emplace_back(
            YSolowork::untility::Device{
                .type = YSolowork::untility::deviceType::Unknown,
                .platformName = "Unknown",
                .name = "Unknown",
                .cores = 0,
                .memoryGB = 0.0
            }
        );
        spdlog::error("Failed to get devices 获取设备信息失败: {}", e.what());
        spdlog::warn("Device related features may not work properly 设备相关功能可能无法正常工作");
    }

    return machineInfo;
}

void logWorkerMachineInfo(const MachineInfo& machineInfo)
{
    spdlog::info("\n---------------------------- Worker Machine Info 工作机器信息 ----------------------------\n\n");
    spdlog::info("\tMachine Name 机器名: {}", machineInfo.machineName);
    spdlog::info("\tSystem 系统: {}", magic_enum::enum_name(machineInfo.systomInfo.os));
    spdlog::info("\tSystem Name 系统名: {}", machineInfo.systomInfo.osName);
    spdlog::info("\tSystem Release 系统发行版: {}", machineInfo.systomInfo.osRelease);
    spdlog::info("\tSystem Version 系统版本号: {}", machineInfo.systomInfo.osVersion);
    spdlog::info("\tSystem Architecture 系统架构: {}", magic_enum::enum_name(machineInfo.systomInfo.osArchitecture));
    spdlog::info("\tDevices 设备:\n");
    for (const auto& device : machineInfo.devices) {
        spdlog::info("\t\tPlatform 平台: {}", device.platformName);
        spdlog::info("\t\tName 名称: {}", device.name);
        spdlog::info("\t\tCores 核心数: {}", device.cores);
        spdlog::info("\t\tMemory 内存: {:.2f} GB", device.memoryGB);
        spdlog::info("\t\tType 类型: {}\n", magic_enum::enum_name(device.type));
    }
    spdlog::info(
        "\n\nYLineWorker needs OpenCL support to get device information, if your device information is not displayed, please check if OpenCL is supported or install runtime Driver.\n"
        "\tGPU: install GPU driver\n"
        "\tx86 CPU: install Intel OpenCL runtime driver (including AMD CPU)\n"
        "\tARM CPU: install ARM OpenCL runtime driver\n"
        "需要 OpenCL 支持以获取设备信息, 如果您的设备信息未显示, 请检查是否支持 OpenCL 或安装运行时驱动程序.\n"
        "\tGPU: 安装 GPU 驱动程序\n"
        "\tx86 CPU: 安装 Intel OpenCL 运行时驱动程序 (包括 AMD CPU)\n"
        "\tARM CPU: 安装 ARM OpenCL 运行时驱动程序\n"
    );
    spdlog::info("\n------------------------------------------------------------------------------------------");
}

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
        .client = client,
        .worker_machineInfo = machineInfo
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

        // 每秒 发送一次 CPU 和 内存 使用率
        auto _usageInfoCPUtimer = loop->runEvery(1.0, [wsClient]() {
            Json::Value json;
            UsageInfoCPU usageInfoCPU = YSolowork::untility::getUsageInfoCPU();
            json["cpuUsage"] = usageInfoCPU.cpuUsage;
            json["memoryUsage"] = usageInfoCPU.memoryUsage;
            if (wsClient)
            {
                wsClient->getConnection()->sendJson(json);
            }
        
        });

        WorkerSingleton::getInstance().usageInfoCPUtimer = _usageInfoCPUtimer;

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
    loop->invalidateTimer(WorkerSingleton::getInstance().usageInfoCPUtimer);
}

void WorkerSingleton::connectToServer() {
    // 获取 client
    auto client = getWorkerData().client;
    // 连接到服务器
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/ws/worker");
    client->setAsyncMessageHandler(msgAsyncCallback);
    client->setConnectionClosedHandler(WSconnectClosedCallback);
    client->connectToServer(
        req,
        WSconnectCallback
    );
}

} // namespace YLineWorker