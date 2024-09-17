#include "logger.h"
#include "UTtime.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>

namespace YLineServer {

// 创建日志记录器
std::shared_ptr<spdlog::logger> createLogger() {
    // 获取当前时间戳作为日志文件名的一部分
    std::string timestamp = YSolowork::untility::getCurrentTimestampStr();
    std::string logFilename = "logs/YLineServer_log_" + timestamp + ".log";

    // 创建控制台输出 sink（带颜色）
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    // 创建文件输出 sink
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilename, true);

    // 创建多 sink logger，将日志同时输出到控制台和文件
    auto logger = std::make_shared<spdlog::logger>("multi_sink", spdlog::sinks_init_list{console_sink, file_sink});

    // 设置日志格式
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

    // 设置日志级别
    logger->set_level(spdlog::level::debug);

    return logger;
}

// 创建一个哈希表来映射字符串到 spdlog 的日志等级
const std::unordered_map<std::string, spdlog::level::level_enum> logLevelMap = {
    {"trace", spdlog::level::trace},
    {"debug", spdlog::level::debug},
    {"info", spdlog::level::info},
    {"warn", spdlog::level::warn},
    {"err", spdlog::level::err},
    {"critical", spdlog::level::critical}
};

}