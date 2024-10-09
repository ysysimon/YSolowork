#ifndef YLINEWORKER_CONFIG_H
#define YLINEWORKER_CONFIG_H

#include <spdlog/spdlog.h>

namespace YLineWorker {

// 结构体: 配置
struct Config {
    // server
    std::string server_ip;
    int server_port;

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

// 单例类: 配置 config
class ConfigSingleton {
public:
    // 获取单例实例的静态方法
    static ConfigSingleton& getInstance();

    // 禁止复制和赋值
    ConfigSingleton(const ConfigSingleton&) = delete;
    ConfigSingleton& operator=(const ConfigSingleton&) = delete;

    // 获取配置数据
    const Config& getConfigData() const;

    // 设置配置数据
    void setConfigData(const Config& configData);

private:
    ConfigSingleton() = default;  // 私有构造函数，防止外部实例化

    Config configData_; // 成员变量，存储配置信息
};


} // namespace YLineWorker

#endif // YLINEWORKER_CONFIG_H