#include "logger.h"
#include "config.h"

int main()
{
    // create logger
    auto logger = YLineServer::createLogger();
    spdlog::set_default_logger(logger);

    spdlog::info("YLineServer starting 启动中...");
    try {
        // parse config file
        YLineServer::Config config = YLineServer::parseConfig();
        spdlog::info("Config file parsed successfully 配置文件解析成功");

        // set log level
        logger->set_level(config.logLevel);
        
        // debug log
        spdlog::debug("Server IP 服务器地址: {}, Port 服务器端口: {}", config.server_ip, config.server_port);
        spdlog::debug("Database Host 数据库地址: {}, Port 数据库端口: {}", config.db_host, config.db_port);

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
    
    return 0;
}