#ifndef YLINEWORKER_APP_H
#define YLINEWORKER_APP_H

#include "utils/config.h"
#include <spdlog/spdlog.h>
#include <memory>

namespace YLineWorker {

void spawnWorker(const Config& config, const std::shared_ptr<spdlog::logger> custom_logger);

}
#endif // YLINEWORKER_APP_H