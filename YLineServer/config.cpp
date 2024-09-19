#include "config.h"
#include "UTfile.h"
#include "logger.h"

#include <spdlog/spdlog.h>
#include <stdexcept>

namespace YLineServer {

Config parseConfig() {
    // get executable path and YLine config path
    std::filesystem::path exePath = YSolowork::untility::getExecutablePath();
    std::filesystem::path YLineServerConfigPath = exePath / "YLineServer_Config.toml";
    spdlog::info("Config file path 配置文件路径: {}", YLineServerConfigPath.string());

    // parse YLine config
    toml::table YLineServerConfig = toml::parse_file(YLineServerConfigPath.string());
    // get YLineServer config
    // 读取 server 部分
    auto &server = *YLineServerConfig["server"].as_table();
    std::string serverIp = server["ip"].value_or("0.0.0.0"); // 默认值 ip
    int serverPort = server["port"].value_or(33383);         // 默认端口

    // 读取 database 部分
    auto &database = *YLineServerConfig["database"].as_table();
    std::string dbHost = database["host"].value_or("localhost");
    int dbPort = database["port"].value_or(5432); // postgresql 默认端口

    // 读取 logger 部分
    auto &loggerTbl = *YLineServerConfig["logger"].as_table();
    std::string logLevelStr = loggerTbl["level"].value_or("debug");

    auto logLevel = spdlog::level::info;
    try {
        logLevel = YLineServer::logLevelMap.at(logLevelStr);
    } 
    catch (const std::out_of_range& e) 
    {
        spdlog::warn("Invalid log level 无效的日志等级: {}", logLevelStr);
        spdlog::warn("Using default log level 使用默认日志等级: info");
    }

    return Config{serverIp, serverPort, dbHost, dbPort, logLevel};
}

} // namespace YLineServer