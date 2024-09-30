#ifndef YLINESERVER_CONFIG_H
#define YLINESERVER_CONFIG_H

#include <cstddef>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <unordered_set>

namespace YLineServer {

// 结构体: 配置
struct Config {
    // server
    std::string server_ip;
    int server_port;

    // middleware
    bool intranet_ip_filter;
    bool local_host_filter;
    bool cors;
    std::unordered_set<std::string> allowed_origins;

    // db
    std::string db_host;
    int db_port;
    std::string db_user;
    std::string db_password;
    std::string db_name;
    size_t db_connection_number;
    float db_timeout;

    // log
    spdlog::level::level_enum log_level;

    // dbmate
    bool migrate;
    std::string dbmate_download_url;
    std::string dbmate_download_name;
    std::filesystem::path dbmate_path;
};

// 函数: 解析配置文件
Config parseConfig();

// 函数: 获取表
toml::table getTable(const std::string& tablename, const toml::table& table);

}

#endif // YLINESERVER_CONFIG_H