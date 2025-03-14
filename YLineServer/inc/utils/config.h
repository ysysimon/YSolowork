#ifndef YLINESERVER_CONFIG_H
#define YLINESERVER_CONFIG_H

#include <chrono>
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
    int server_thread_num;

    // worker
    std::string register_secret;
    std::uint32_t consumer_AMQP_connection;

    // middleware
    bool intranet_ip_filter;
    bool local_host_filter;
    bool cors;
    std::unordered_set<std::string> allowed_origins;

    // jwt
    std::string jwt_secret;
    bool jwt_expire;
    std::chrono::seconds jwt_expire_time;

    // db
    std::string db_host;
    int db_port;
    std::string db_user;
    std::string db_password;
    std::string db_name;
    size_t db_connection_number;
    float db_timeout;

    // redis
    std::string redis_host;
    int redis_port;
    std::string redis_password;
    int redis_index;
    size_t redis_connection_number;
    float redis_timeout;

    // RabbitMQ
    std::string amqp_host;
    int amqp_port;
    std::string amqp_user;
    std::string amqp_password;

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

} // namespace YLineServer

#endif // YLINESERVER_CONFIG_H