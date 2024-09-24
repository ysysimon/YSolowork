#include "config.h"
#include "UTfile.h"
#include "logger.h"

#include <cstddef>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace YLineServer {

Config parseConfig() {
    // get executable path and YLine config path
    const std::filesystem::path& exePath = YSolowork::untility::getExecutablePath();
    const std::filesystem::path& YLineServerConfigPath = exePath / "YLineServer_Config.toml";
    spdlog::info("Config file path 配置文件路径: {}", YLineServerConfigPath.string());

    // parse YLine config
    toml::table YLineServerConfig = toml::parse_file(YLineServerConfigPath.string());
    // get YLineServer config
    // 读取 server 部分
    auto &server = *YLineServerConfig["server"].as_table();
    const std::string& serverIp = server["ip"].value_or("0.0.0.0"); // 默认值 ip
    int serverPort = server["port"].value_or(33383);         // 默认端口

    // 读取 database 部分
    auto &database = *YLineServerConfig["database"].as_table();
    const std::string& dbHost = database["host"].value_or("localhost");
    int dbPort = database["port"].value_or(5432); // postgresql 默认端口
    const std::string& dbUser = database["db_user"].value_or("postgres");
    const std::string& dbPassword = database["db_password"].value_or("postgresPassword");
    const std::string& dbName = database["db_name"].value_or("yline");
    size_t dbConnectionNumber = database["connection_number"].value_or(10);
    float dbTimeout = database["timeout"].value_or(5.0);

    // 读取 logger 部分
    auto &loggerTbl = *YLineServerConfig["logger"].as_table();
    const std::string& logLevelStr = loggerTbl["level"].value_or("debug");
    auto logLevel = spdlog::level::info;
    try {
        logLevel = YLineServer::logLevelMap.at(logLevelStr);
    } 
    catch (const std::out_of_range& e) 
    {
        spdlog::warn("Invalid log level 无效的日志等级: {}", logLevelStr);
        spdlog::warn("Using default log level 使用默认日志等级: info");
    }

    // 读取 dbmate 部分
    bool migration = YLineServerConfig["dbmate"]["migration"].value_or(true);
    const std::string& dbmate_download_url = YLineServerConfig["dbmate"]["download_url"].value_or("No dbmate download url!");
    
    #if defined(__linux__)
        const std::string& dbmate_download_name = YLineServerConfig["dbmate"]["linux_name"].value_or("dbmate-linux-amd64");
    #elif defined(_WIN32) || defined(_WIN64)
        const std::string& dbmate_download_name = YLineServerConfig["dbmate"]["win_name"].value_or("dbmate-windows-amd64.exe");
    #endif

    return Config{
        serverIp, 
        serverPort, 
        dbHost, 
        dbPort, 
        dbUser,
        dbPassword,
        dbName,
        dbConnectionNumber,
        dbTimeout,
        logLevel,
        migration,
        dbmate_download_url,
        dbmate_download_name,
        exePath
        };
}

} // namespace YLineServer