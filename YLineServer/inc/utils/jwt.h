#ifndef YLINESERVER_JWT_H
#define YLINESERVER_JWT_H

#include "models/Users.h"
#include <json/value.h>
#include <tl/expected.hpp>

#include "utils/server.h"
#include <system_error>

namespace YLineServer::Jwt {

using namespace drogon_model::yline;

// ------------------- 错误处理 -------------------
// Jwt 错误码
enum class JwtError {
    JwtFormatError, // Token is not in correct format
    JwtDecodingError, // Base64 decoding failed or invalid json
    JwtInvalidSignature, // Signature verification failed
    JwtEncryptionError, // Encryption failed
    JwtVerificationError, // Verification failed
    UnknownError
};
// Jwt 错误码类别
class JwtErrorCategory : public std::error_category {
public:
    const char* name() const noexcept override {
        return "JwtError";
    }
    std::string message(int ev) const override {
        switch (static_cast<JwtError>(ev)) {
        case JwtError::JwtFormatError:
            return "Token is not in correct format";
        case JwtError::JwtDecodingError:
            return "Base64 decoding failed or invalid json";
        case JwtError::JwtInvalidSignature:
            return "Signature verification failed";
        case JwtError::JwtEncryptionError:
            return "Encryption failed";
        case JwtError::JwtVerificationError:
            return "Verification failed";
        case JwtError::UnknownError:
            return "Unknown error";
        default:
            return "Unrecognized JWT error code";
        }
    }
};

// Jwt 错误码类别全局实例
inline const JwtErrorCategory& jwt_error_category()
{
    static JwtErrorCategory instance;
    return instance;
}

// Jwt 错误码工厂函数
inline std::error_code make_error_code(JwtError e) noexcept
{
    return {static_cast<int>(e), jwt_error_category()};
}



// ---------------------------------------------


// 函数: 生成 JWT
std::string generateAuthJwt(const Users::PrimaryKeyType& userId, const std::string& username, const bool isAdmin);

// 函数: 解码 JWT
bool decodeAuthJwt(const std::string& token, Json::Value& payload, std::string& err);

// 函数: 验证 Bearer JWT
bool validateBearerToken(const std::string &authHeader, Json::Value& payload, std::string& err);

// 函数: 验证 JWT (非Bearer) 成功则返回用户实体 ID
tl::expected<EnTTidType, std::error_code> verifyToken2EnTTUser(const std::string& token) noexcept;

}


// 兼容 std::error_code
namespace std {
    template <>
    struct is_error_code_enum<YLineServer::Jwt::JwtError> : true_type {};
}

#endif // YLINESERVER_JWT_H