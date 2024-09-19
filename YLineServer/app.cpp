#include "app.h"
#include "logger.h"

#include <spdlog/spdlog.h>
#include <drogon/drogon.h>
#include <trantor/utils/Logger.h>

namespace YLineServer {

void spawnApp(const Config& config, const std::shared_ptr<spdlog::logger> custom_logger)
{
    // 检查是否支持 spdlog
    // 注意: 需要使用 开启了 spdlog 支持的 Drogon 构建
    if (!trantor::Logger::hasSpdLogSupport()) {
        // throw exception
        throw std::runtime_error("Current Drogon build does not support spdlog. 当前 Drogon 构建不支持 spdlog.");
    }

    // set drogon logger
    trantor::Logger::enableSpdLog(custom_logger);
    trantor::Logger::setLogLevel(YLineServer::spdlogToDrogonLogLevel.at(config.logLevel));

    drogon::app().addListener(config.server_ip, config.server_port);
    LOG_INFO << "Start Listening 开始监听";
    drogon::app().run();

}

}