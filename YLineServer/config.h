#ifndef YLINESERVER_CONFIG_H
#define YLINESERVER_CONFIG_H

#include <spdlog/spdlog.h>

namespace YLineServer {

// 结构体: 配置
struct Config {
    std::string server_ip;
    int server_port;
    std::string db_host;
    int db_port;
    spdlog::level::level_enum logLevel;
};

// 函数: 解析配置文件
Config parseConfig();

}

#endif // YLINESERVER_CONFIG_H