#include "utils/config.h"
#include "UTfile.h"
#include "utils/logger.h"
#include "vendor_include/toml.hpp"

#include <cstddef>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>

namespace YLineServer {

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
    const std::filesystem::path& exePath = YSolowork::util::getExecutablePath();
    const std::filesystem::path& YLineServerConfigPath = exePath / "YLineServer_Config.toml";
    spdlog::info("Config file path 配置文件路径: {}", YLineServerConfigPath.string());
    spdlog::info(
        "\n----------Start to parse YLineServer config file 开始解析 YLineServer 配置文件----------\n"
        );

    // parse YLine config
    toml::table YLineServerConfig = toml::parse_file(YLineServerConfigPath.string());

    // get YLineServer config
    // 读取 server 部分
    const auto& server = getTable("server", YLineServerConfig);
    const std::string& serverIp = server["ip"].value_or("0.0.0.0"); // 默认值 ip
    int serverPort = server["port"].value_or(33383);         // 默认端口
    int serverThreadNum = server["work_thread"].value_or(4); // 默认线程数

    // 读取 worker 部分
    const auto& worker = getTable("worker", YLineServerConfig);
    const std::string& registerSecret = worker["register_secret"].value_or("");
    if (registerSecret.empty()) 
    {
        spdlog::error("Worker Register secret is empty 工作机注册密钥为空");
        throw std::runtime_error("Register secret is empty");
    }
    std::uint32_t consumerAMQPConnection = worker["consumer_AMQP_connection"].value_or(1); // 默认 4

    // 读取 middleware 部分
    const auto& middleware = getTable("middleware", YLineServerConfig);
    bool intranetIpFilter = middleware["IntranetIpFilter"].value_or(false);
    bool localHostFilter = middleware["LocalHostFilter"].value_or(false);
    // 跨域部分 CORS
    bool cors = middleware["CORS"].value_or(false);
    std::unordered_set<std::string> allowedOrigins;
    if (cors) {
        spdlog::info("CORS enabled 跨域请求已启用");
        if (!middleware["CORSAllowOrigins"].as_array())
        {
            spdlog::error("CORSAllowOrigins is not an array 不是一个数组");
            throw std::runtime_error("CORSAllowOrigins is not an array");
        }
        const auto& corsTbl = *middleware["CORSAllowOrigins"].as_array();
        std::string allowedOriginsStr;
        std::string allowedOriginStrs = "";
        int index = 0;
        for (const auto &origin : corsTbl) {
            // 使用 value_or("") 来安全获取字符串，默认为空字符串
            allowedOriginsStr = origin.value_or<std::string>("");
            allowedOrigins.insert(allowedOriginsStr);
            allowedOriginStrs += std::format("\t[{}] - {}\n", index++, allowedOriginsStr);
        }
        spdlog::info("Allowed origins 允许的跨域来源:\n\n {}", allowedOriginStrs);
    }

    // 读取 jwt 部分
    const auto& jwt = getTable("jwt", YLineServerConfig);
    const std::string& jwtSecret = jwt["secret"].value_or("");
    if (jwtSecret.empty()) {
        spdlog::error("JWT secret is empty JWT 密钥为空");
        throw std::runtime_error("JWT secret is empty");
    }
    bool jwtexpire = jwt["expire"].value_or(true);
    std::chrono::seconds jwtexpireTime = std::chrono::seconds(jwt["expire_time"].value_or(432000)); // 默认 5 天

    // 读取 database 部分
    const auto& database = getTable("database", YLineServerConfig);
    const std::string& dbHost = database["host"].value_or("localhost");
    int dbPort = database["port"].value_or(5432); // postgresql 默认端口
    const std::string& dbUser = database["db_user"].value_or("postgres");
    const std::string& dbPassword = database["db_password"].value_or("postgresPassword");
    const std::string& dbName = database["db_name"].value_or("yline");
    size_t dbConnectionNumber = database["connection_number"].value_or(10);
    float dbTimeout = database["timeout"].value_or(5.0);

    // 读取 redis 部分
    const auto& redis = getTable("redis", YLineServerConfig);
    const std::string& redisHost = redis["host"].value_or("127.0.0.1");
    int redisPort = redis["port"].value_or(6379); // redis 默认端口
    const std::string& redisPassword = redis["password"].value_or("");
    int redisIndex = redis["index"].value_or(0);
    size_t redisConnectionNumber = redis["connection_number"].value_or(10);
    float redisTimeout = redis["timeout"].value_or(5.0);

    // 读取 RabbitMQ 部分
    const auto& amqp = getTable("RabbitMQ", YLineServerConfig);
    const std::string& amqpHost = amqp["host"].value_or("127.0.0.1");
    int amqpPort = amqp["port"].value_or(5672); // RabbitMQ 默认端口
    const std::string& amqpUser = amqp["username"].value_or("guest");
    const std::string& amqpPassword = amqp["password"].value_or("guest");

    // 读取 logger 部分
    const auto& loggerTbl = getTable("logger", YLineServerConfig);
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

    spdlog::info(
        "\n----------End of parsing YLineServer config file 解析 YLineServer 配置文件结束----------\n"
        );
    return Config{
        serverIp, 
        serverPort,
        serverThreadNum,
        registerSecret,
        consumerAMQPConnection,
        intranetIpFilter,
        localHostFilter,
        cors,
        allowedOrigins,
        jwtSecret,
        jwtexpire,
        jwtexpireTime,
        dbHost, 
        dbPort, 
        dbUser,
        dbPassword,
        dbName,
        dbConnectionNumber,
        dbTimeout,
        redisHost,
        redisPort,
        redisPassword,
        redisIndex,
        redisConnectionNumber,
        redisTimeout,
        amqpHost,
        amqpPort,
        amqpUser,
        amqpPassword,
        logLevel,
        migration,
        dbmate_download_url,
        dbmate_download_name,
        exePath
        };
}

} // namespace YLineServer