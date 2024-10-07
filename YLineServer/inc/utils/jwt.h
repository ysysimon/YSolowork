#ifndef YLINESERVER_JWT_H
#define YLINESERVER_JWT_H

#include "models/Users.h"

namespace YLineServer::Jwt {

using namespace drogon_model::yline;

// 函数: 生成 JWT
std::string generateAuthJwt(const Users::PrimaryKeyType& userId, const std::string& username, const bool isAdmin);

}

#endif // YLINESERVER_JWT_H