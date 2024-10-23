#ifndef YLINEWORKER_CONFIG_H
#define YLINEWORKER_CONFIG_H

#include <spdlog/spdlog.h>

namespace YLineWorker {

// 结构体: 配置
struct Config {
    // YLineWorker
    std::string YLineWorker_ip;
    int YLineWorker_port;

    // worker
    std::string register_secret;

    // YLineServer
    std::string YLineServer_ip;
    int YLineServer_port;

    // middleware
    bool intranet_ip_filter;
    bool local_host_filter;

    // log
    spdlog::level::level_enum log_level;
};

// 函数: 解析配置文件
Config parseConfig();

// 函数: 获取表
toml::table getTable(const std::string& tablename, const toml::table& table);

} // namespace YLineWorker

#endif // YLINEWORKER_CONFIG_H