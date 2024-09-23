#include "database.h"
#include <string>

#include "UTdbmate.h"

namespace YLineServer {

std::string getPostgresConnectionString(const Config& config) {
    std::string conn_str = "postgres://username:password@host:port/database";
    return conn_str;

}

void downloadDBMATEifNotExist(const Config& config) {
    // download dbmate if not exist
    if (!YSolowork::untility::isDbmateInstalled(config.dbmate_path)) {
        spdlog::info("Downloading dbmate... 下载 dbmate...");
        YSolowork::untility::downloadDbmate(config.dbmate_download_url, config.dbmate_download_name, config.dbmate_path);
    }
}

}