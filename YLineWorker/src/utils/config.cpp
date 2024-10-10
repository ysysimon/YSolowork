#include "utils/config.h"
#include "UTfile.h"
#include "utils/logger.h"
#include "vendor_include/toml.hpp"


#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>

namespace YLineWorker {

toml::table getTable(const std::string& tablename, const toml::table& table) {
    if (table.contains(tablename) == false) {
        spdlog::error("Table {} not found in config file 配置文件中没有找到 {} 项", tablename, tablename);
        throw std::runtime_error("Table not found in config file");
    }

    if (!table[tablename].as_table())
    {
        spdlog::error("Table {} is not a table 不是一个表", tablename, tablename);
        throw std::runtime_error("Table is not a table");
    }

    return *table[tablename].as_table();
    
}

Config parseConfig() {
    // get executable path and YLine config path
    const std::filesystem::path& exePath = YSolowork::untility::getExecutablePath();
    const std::filesystem::path& YLineWorkerConfigPath = exePath / "YLineWorker_Config.toml";
    spdlog::info("Config file path 配置文件路径: {}", YLineWorkerConfigPath.string());
    spdlog::info(
        "\n----------Start to parse YLineServer config file 开始解析 YLineWorker 配置文件----------\n"
        );

    // parse YLine config
    toml::table YLineWorkerConfig = toml::parse_file(YLineWorkerConfigPath.string());

    // get YLineWorker config
    // 读取 YLineWorker 部分
    const auto& YLineWorker = getTable("YLineWorker", YLineWorkerConfig);
    const std::string& YLineWorkerIp = YLineWorker["ip"].value_or("0.0.0.0"); // 默认值 ip
    int YLineWorkerPort = YLineWorker["port"].value_or(33393);         // 默认端口

    // 读取 YLineServer 部分
    const auto& YLineServer = getTable("YLineServer", YLineWorkerConfig);
    const std::string& YLineServerIp = YLineServer["ip"].value_or("0.0.0.0");
    int YLineServerPort = YLineServer["port"].value_or(33383);

    // 读取 middleware 部分
    const auto& middleware = getTable("middleware", YLineWorkerConfig);
    bool intranetIpFilter = middleware["IntranetIpFilter"].value_or(false);
    bool localHostFilter = middleware["LocalHostFilter"].value_or(false);

    // 读取 logger 部分
    const auto& loggerTbl = getTable("logger", YLineWorkerConfig);
    const std::string& logLevelStr = loggerTbl["level"].value_or("debug");
    auto logLevel = spdlog::level::info;
    try {
        logLevel = YLineWorker::logLevelMap.at(logLevelStr);
    } 
    catch (const std::out_of_range& e) 
    {
        spdlog::warn("Invalid log level 无效的日志等级: {}", logLevelStr);
        spdlog::warn("Using default log level 使用默认日志等级: info");
    }

    spdlog::info(
        "\n----------End of parsing YLineServer config file 解析 YLineWorker 配置文件结束----------\n"
        );
    return Config{
        YLineWorkerIp,
        YLineWorkerPort,
        YLineServerIp,
        YLineServerPort,
        intranetIpFilter,
        localHostFilter,
        logLevel
        };
}

// 单例类: 配置 config 定义

ConfigSingleton& ConfigSingleton::getInstance() 
{
    static ConfigSingleton instance; // 单例实例
    return instance;
}

const Config& ConfigSingleton::getConfigData() const
{
    return configData_;
}

void ConfigSingleton::setConfigData(const Config& configData) 
{
    configData_ = configData;
}

} // namespace YLineWorker