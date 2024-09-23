#include "database.h"
#include <string>
#include <format>

#include "UTdbmate.h"


namespace YLineServer {

std::string getPostgresConnectionString(const Config& config) {
    // std::string conn_str = "postgres://username:password@host:port/database";
    return std::format(
        "postgres://{}:{}@{}:{}/{}",
        config.db_user, config.db_password, config.db_host, config.db_port, config.db_name
    );

}

void downloadDBMATEifNotExist(const Config& config) {
    // download dbmate if not exist
    if (!YSolowork::untility::isDbmateInstalled(config.dbmate_path)) {
        spdlog::info("Downloading dbmate... 下载 dbmate...");
        YSolowork::untility::downloadDbmate(config.dbmate_download_url, config.dbmate_download_name, config.dbmate_path);
    }
}

drogon::orm::DbConfig getDrogonPostgresConfig (const Config& config) {

    return drogon::orm::PostgresConfig{
        .host = config.db_host,
        .port = static_cast<unsigned short>(config.db_port),
        .databaseName = config.db_name,
        .username = config.db_user,
        .password = config.db_password,
        .connectionNumber = config.db_connection_number,
        .name = "YLinedb",
        .isFast = true, // use fast mode, need to use DB inside event loop
        .characterSet = "UTF8",
        .timeout = config.db_timeout,
        .autoBatch = true,
        .connectOptions = {}
    };
}

}