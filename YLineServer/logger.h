#ifndef YLINESERVER_LOGGER_H
#define YLINESERVER_LOGGER_H

#include <spdlog/spdlog.h>

namespace YLineServer {

// 函数: 创建日志记录器
std::shared_ptr<spdlog::logger> createLogger();

// 哈希表: 映射字符串到 spdlog 的日志等级
extern const std::unordered_map<std::string, spdlog::level::level_enum> logLevelMap;

}

#endif // YLINESERVER_LOGGER_H