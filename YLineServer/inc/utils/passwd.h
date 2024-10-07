#ifndef YLINESERVER_PASSWD_H
#define YLINESERVER_PASSWD_H

namespace YLineServer::Passwd {

// 函数: 生成盐
std::string generateSalt(size_t length = 16);

// 函数: 哈希密码
std::string hashPassword(const std::string& password, const std::string& salt, const std::string& ALGORITHM = "SHA-256");

// 函数: 比较哈希
bool compareHash(const std::string& storedHash, const std::string& inputPasswordHash);

}

#endif // "YLINESERVER_PASSWD_H"