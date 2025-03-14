#include "utils/logger.h"
#include "utils/config.h"
#include "utils/server.h"

#include "app.h"
#include "database.h"

#include "UTconsoleUTF8.h"

int main()
{
    // set console to UTF-8
    YSolowork::util::setConsoleUTF8();

    // create logger
    auto logger = YLineServer::createLogger();

    spdlog::info("YLineServer starting 启动中...");
    try {
        // parse config file
        YLineServer::Config config = YLineServer::parseConfig();
        spdlog::info("Config file parsed successfully 配置文件解析成功");
        // 构造单例 服务器类
        YLineServer::ServerSingleton::getInstance().setConfigData(config);

        // set log level
        logger->set_level(config.log_level);
        // log level output here
        spdlog::info("Log level 日志等级: {}", YLineServer::reverseLogLevelMap.at(logger->level()));
        
        // debug log
        spdlog::debug("Server IP 服务器地址: {}, Port 服务器端口: {}", config.server_ip, config.server_port);
        spdlog::debug("Database Host 数据库地址: {}, Port 数据库端口: {}", config.db_host, config.db_port);
        spdlog::debug("Postgres url: {}", YLineServer::DB::getPostgresConnectionString(config));
        spdlog::debug("DB Connection Number 数据库连接数: {}", config.db_connection_number);
        spdlog::debug("DB Timeout 数据库超时时间: {}", config.db_timeout);

        // dbmate for database migration
        YLineServer::DB::downloadDBMATEifNotExist(config);

        // database
        YLineServer::DB::initDataBasePostgres(config); // init database
        YLineServer::DB::migrateDatabase(config); // migrate database

        // start server
        YLineServer::spawnApp(config, logger);
        spdlog::info("YLineServer stopped 停止");
    } 
    catch (const toml::parse_error& err)
    {
        spdlog::critical("Config Parsing failed 配置文件解析失败:{}", err.what());
        return EXIT_FAILURE;
    }
    catch (const std::exception& e) {
        spdlog::critical("Unknown 未知错误:{}", e.what());
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}