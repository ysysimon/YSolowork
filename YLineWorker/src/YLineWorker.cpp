#include "UTconsoleUTF8.h"
#include "UTsysmutex.h"
#include "utils/logger.h"
#include "utils/config.h"
#include "worker.h"

int main() {
  // set console to UTF-8
  YSolowork::util::setConsoleUTF8();

  // get mutex
  YSolowork::util::CrossPlatformSysMutex Sysmutex("YLineWorker");

  if (!Sysmutex.try_lock()) {
    spdlog::critical("Worker instance already exists 工人实例已经存在");
    return EXIT_FAILURE;
  }
  
  try {
    // create logger
    auto logger = YLineWorker::createLogger();
    spdlog::info("YLineWorker Service starting 启动中...");

    // parse config file
    YLineWorker::Config config = YLineWorker::parseConfig();
    spdlog::info("Config file parsed successfully 配置文件解析成功");

    // set log level
    logger->set_level(config.log_level);
    // log level output here
    spdlog::info("Log level 日志等级: {}", YLineWorker::reverseLogLevelMap.at(logger->level()));

    // debug log
    spdlog::debug("YLineWorker Service IP 服务器地址: {}, Port 服务器端口: {}", config.YLineWorker_ip, config.YLineWorker_port);

    // start server
    YLineWorker::spawnWorker(config, logger);
    spdlog::info("YLineWorker Service stopped 停止");
  } 
  catch (const toml::parse_error& err) {
    spdlog::critical("Config Parsing failed 配置文件解析失败:{}", err.what());
    Sysmutex.unlock();
    return EXIT_FAILURE;
  } 
  // catch (const YSolowork::util::CrossPlatformSysMutexError& e) {
  //   spdlog::critical("Mutex error Worker instance may already exist, 互斥量错误, 工人实例可能已经存在:{}", e.what());
  //   return EXIT_FAILURE;
  // }
  catch (const std::exception& e) {
    spdlog::critical("Unknown 未知错误:{}", e.what());
    Sysmutex.unlock();
    return EXIT_FAILURE;
  }

  Sysmutex.unlock();

  return EXIT_SUCCESS;
}