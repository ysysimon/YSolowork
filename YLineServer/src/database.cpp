#include "database.h"
#include <stdexcept>
#include <string>
#include <format>

#include "UTdbmate.h"
#include "spdlog/spdlog.h"

namespace YLineServer::DB {



std::string getPostgresConnectionString(const Config& config) {
    // std::string conn_str = "postgres://username:password@host:port/database";
    return std::format(
        "postgres://{}:{}@{}:{}/{}?sslmode=disable",
        config.db_user, config.db_password, config.db_host, config.db_port, config.db_name
    );

}

void initDataBasePostgres(const Config& config) {

    try {
        // 注意这不是真正用于数据库的连接池，只是在 Windows 上无法在安装 PgSQL 时指定数据库名称
        // 所以我们需要进行一次连接以创建数据库，完成后这个 dbclient 就会被销毁
        spdlog::info("Initializing database 初始化数据库...");
        const auto PgClient = getTempPgClient(
            config.db_host, config.db_port, "postgres", config.db_user, config.db_password
        );

        // 查询是否存在目标数据库
        const auto& result = PgClient->execSqlSync(
            std::format("SELECT 1 FROM pg_database WHERE datname='{}'", config.db_name)
        );

        // 如果数据库不存在，则创建它
        if (result.empty()) {
            PgClient->execSqlSync(std::format("CREATE DATABASE {}", config.db_name));
            spdlog::info("Database '{}' created successfully 数据库创建成功.", config.db_name);
        } else {
            spdlog::info("Database '{}' already exists 数据库已存在.", config.db_name);
        }
    } catch (const drogon::orm::DrogonDbException& e) {
        throw std::runtime_error(
            std::format("Database initialization failed 数据库初始化失败 - {}", e.base().what())
        );
    }

}

void downloadDBMATEifNotExist(const Config& config) {
    // download dbmate if not exist
    if (!YSolowork::util::isDbmateInstalled(config.dbmate_path)) {
        spdlog::info("Downloading dbmate... 下载 dbmate...");
        YSolowork::util::downloadDbmate(config.dbmate_download_url, config.dbmate_download_name, config.dbmate_path);
    }
}

drogon::orm::PostgresConfig getDrogonPostgresConfig (const Config& config) {

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

void migrateDatabase(const Config& config) {
    spdlog::info("Migrating database... 数据库迁移...");
    try {
        spdlog::debug("Dbmate Path: {}", (config.dbmate_path / YSolowork::util::DBMATE_BINARY).string());
        const auto& [result_log, exit_code] = YSolowork::util::runDbmate(config.dbmate_path, "up", getPostgresConnectionString(config));
        spdlog::info("Migration log 迁移日志: {}", result_log);
        if (exit_code != 0) {
            throw;
        }
        spdlog::info("Database migrated successfully 数据库迁移成功.");
    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::format("Database migration failed 数据库迁移失败 - {}", e.what())
        );
    }
}

} // namespace YLineServer::DB