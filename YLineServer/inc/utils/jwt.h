#ifndef YLINESERVER_JWT_H
#define YLINESERVER_JWT_H

#include "models/Users.h"
#include <json/value.h>

namespace YLineServer::Jwt {

using namespace drogon_model::yline;

// 函数: 生成 JWT
std::string generateAuthJwt(const Users::PrimaryKeyType& userId, const std::string& username, const bool isAdmin);

// 函数: 解码 JWT
bool decodeAuthJwt(const std::string& token, Json::Value& payload, std::string& err);

}

#endif // YLINESERVER_JWT_H