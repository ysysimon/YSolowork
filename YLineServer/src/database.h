#ifndef YLINESERVER_DATABASE_H
#define YLINESERVER_DATABASE_H

#include <string>

#include "utils/config.h"
#include <drogon/orm/DbConfig.h>
#include <drogon/orm/DbClient.h>

namespace YLineServer::DB {

// 函数: 获取 Postgres 连接字符串
std::string getPostgresConnectionString(const Config& config);

// 函数: 如果不存在则下载 dbmate
void downloadDBMATEifNotExist(const Config& config);

// 函数: 获取 Drogon Postgres 配置
drogon::orm::PostgresConfig getDrogonPostgresConfig (const Config& config);

// 函数: 初始化数据库
void initDataBasePostgres(const Config& config);

// 函数: 迁移数据库
void migrateDatabase(const Config& config);

// 函数: 获取一个 PgClient 以供临时使用
inline std::shared_ptr<drogon::orm::DbClient> getTempPgClient
(
    const std::string & db_host,
    const int db_port,
    const std::string & db_name,
    const std::string & db_user,
    const std::string & db_password
)
{
    return drogon::orm::DbClient::newPgClient
    (
        std::format
        (
            "host={} port={} dbname={} user={} password={}",
            db_host, db_port, db_name, db_user, db_password
        ), // connInfo
        1 // connection number
    );
}

}

#endif // YLINESERVER_DATABASE_H