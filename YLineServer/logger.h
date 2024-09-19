#ifndef YLINESERVER_LOGGER_H
#define YLINESERVER_LOGGER_H

#include <spdlog/spdlog.h>
#include <trantor/utils/Logger.h>

namespace YLineServer {

// 函数: 创建日志记录器
std::shared_ptr<spdlog::logger> createLogger();

// 哈希表: 映射字符串到 spdlog 的日志等级
extern const std::unordered_map<std::string, spdlog::level::level_enum> logLevelMap;

// 反向映射：从 spdlog 日志级别到字符串
extern const std::unordered_map<spdlog::level::level_enum, const std::string> reverseLogLevelMap;

// 哈希表: 将 spdlog 的日志级别映射到 Drogon/Trantor 的日志级别
extern const std::unordered_map<spdlog::level::level_enum, trantor::Logger::LogLevel> spdlogToDrogonLogLevel;

}

#endif // YLINESERVER_LOGGER_H