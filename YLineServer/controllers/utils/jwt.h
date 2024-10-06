#ifndef YLINESERVER_JWT_H
#define YLINESERVER_JWT_H

namespace YLineServer::Jwt {

// 函数: 生成 JWT
std::string generateAuthJwt(const std::string& userId, const std::string& username, const bool isAdmin);

}

#endif // YLINESERVER_JWT_H