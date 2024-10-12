#include "UTconsoleUTF8.h"
#include "UTnvml.h"
#include "utils/logger.h"
#include "utils/config.h"
#include "worker.h"

int main() {
  // set console to UTF-8
  YSolowork::untility::setConsoleUTF8();
  
  // create logger
  auto logger = YLineWorker::createLogger();

  spdlog::info("YLineWorker Service starting 启动中...");
  try {
    // parse config file
    YLineWorker::Config config = YLineWorker::parseConfig();
    spdlog::info("Config file parsed successfully 配置文件解析成功");
    // 构造单例 配置类
    YLineWorker::ConfigSingleton::getInstance().setConfigData(config);

    // set log level
    logger->set_level(config.log_level);
    // log level output here
    spdlog::info("Log level 日志等级: {}", YLineWorker::reverseLogLevelMap.at(logger->level()));

    // debug log
    spdlog::debug("YLineWorker Service IP 服务器地址: {}, Port 服务器端口: {}", config.YLineWorker_ip, config.YLineWorker_port);

    // test nvml
    spdlog::info("Test NVML 测试 NVML, GPU Name 显卡名称: {}", YSolowork::untility::test_nvml());

    // start server
    YLineWorker::spawnWorker(config, logger);
    spdlog::info("YLineWorker Service stopped 停止");
  } 
  catch (const toml::parse_error& err) {
    spdlog::critical("Config Parsing failed 配置文件解析失败:{}", err.what());
    return EXIT_FAILURE;
  } 
  catch (const std::exception& e) {
    spdlog::critical("Unknown 未知错误:{}", e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}