#ifndef YLINESERVER_DATABASE_H
#define YLINESERVER_DATABASE_H

#include <string>

#include "config.h"
#include <drogon/orm/DbConfig.h>

namespace YLineServer {

// 函数: 获取 Postgres 连接字符串
std::string getPostgresConnectionString(const Config& config);

// 函数: 如果不存在则下载 dbmate
void downloadDBMATEifNotExist(const Config& config);

// 函数: 获取 Drogon Postgres 配置
drogon::orm::DbConfig getDrogonPostgresConfig (const Config& config);

}

#endif // YLINESERVER_DATABASE_H