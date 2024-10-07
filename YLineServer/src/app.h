#ifndef YLINESERVER_APP_H
#define YLINESERVER_APP_H

#include "utils/config.h"
#include <spdlog/spdlog.h>
#include <memory>

namespace YLineServer {

void spawnApp(const Config& config, const std::shared_ptr<spdlog::logger> custom_logger);

}
#endif // YLINESERVER_APP_H