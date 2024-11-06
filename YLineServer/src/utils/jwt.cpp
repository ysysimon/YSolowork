#include "utils/jwt.h"
#include "jwt-cpp/jwt.h"
#include "utils/server.h"

#include <json/value.h>
#include "jwt-cpp/traits/open-source-parsers-jsoncpp/traits.h"
#include <json/json.h>
#include <spdlog/spdlog.h> // for testing

namespace YLineServer::Jwt {

using traits = jwt::traits::open_source_parsers_jsoncpp;
using claim = jwt::basic_claim<traits>;


std::string generateAuthJwt(const Users::PrimaryKeyType& userId, const std::string& username, const bool isAdmin)
{
    const ServerSingleton& config = ServerSingleton::getInstance();
    const std::string& jwtSecret = config.getConfigData().jwt_secret;
    auto token = jwt::create<traits>()
        .set_issuer("YLineServer")
        .set_type("JWT")
        .set_issued_now()
        .set_payload_claim("userId", userId)
        .set_payload_claim("username", username)
        .set_payload_claim("isAdmin", isAdmin);
    
    if (config.getConfigData().jwt_expire) {
        const std::chrono::seconds& jwtExpireTime = config.getConfigData().jwt_expire_time;
        token.set_expires_in(jwtExpireTime);
    }

    // no need to send secret to client, so we can use HS256, and it's good for Hexpansion
    return token.sign(jwt::algorithm::hs256{ jwtSecret });
}

bool decodeAuthJwt(const std::string& token, Json::Value& payload, std::string& err)
{
    const ServerSingleton& config = ServerSingleton::getInstance();
    const std::string& jwtSecret = config.getConfigData().jwt_secret;
    try {
        auto decoded = jwt::decode<traits>(token);
        // 创建验证器
        auto verifier = jwt::verify<traits>()
            .allow_algorithm(jwt::algorithm::hs256{ jwtSecret })
            .with_issuer("YLineServer");
        // 验证JWT的有效性
        verifier.verify(decoded);
        
        // 从JWT中提取载荷
        payload = decoded.get_payload_json();

        return true;
    } catch (const std::exception& e) {
        err = e.what();
        return false;
    }
}

bool validateBearerToken(const std::string &authHeader, Json::Value& payload, std::string& err)
{
    // 从Authorization头中提取JWT
    if (authHeader.find("Bearer ") != 0)
        return false;

    const std::string& token = authHeader.substr(7);  // 去掉"Bearer "部分

    // 验证 JWT
    bool valid = Jwt::decodeAuthJwt(token, payload, err);

    return valid;
}

tl::expected<EnTTidType, std::error_code> verifyToken2EnTTUser(const std::string& token) noexcept
{
    const std::string& jwtSecret = ServerSingleton::getInstance().getConfigData().jwt_secret;

    // 解码并验证 JWT
    try 
    {
        const auto& decoded = jwt::decode<traits>(token);
        auto verifier = jwt::verify<traits>()
            .allow_algorithm(jwt::algorithm::hs256{ jwtSecret })
            .with_issuer("YLineServer");
        verifier.verify(decoded);

        // 从 JWT 中提取载荷
        const Json::Value& payload = decoded.get_payload_json();
        // spdlog::debug("payload: {}", payload.toStyledString());
        if (
            !payload.isMember("userId") ||
            !payload.isMember("username") ||
            !payload.isMember("isAdmin")
        )
            return tl::make_unexpected(make_error_code(JwtError::JwtExtractPayloadError));

        const auto& User = ServerSingleton::getInstance().addUserEntt(
            static_cast<Users::PrimaryKeyType>(payload["userId"].asUInt64()),
            payload["username"].asString(),
            payload["isAdmin"].asBool()
        );

        return User;
        
    } 
    catch (jwt::error::rsa_exception &e) 
    {
        return tl::make_unexpected(make_error_code(JwtError::JwtEncryptionError));
    }
    catch (jwt::error::ecdsa_exception &e)
    {
        return tl::make_unexpected(make_error_code(JwtError::JwtEncryptionError));
    }
    catch (jwt::error::token_verification_exception &e)
    {
        return tl::make_unexpected(make_error_code(JwtError::JwtVerificationError));
    }
    catch (jwt::error::signature_verification_error &e)
    {
        return tl::make_unexpected(make_error_code(JwtError::JwtInvalidSignature));
    }
    catch (std::invalid_argument &e) 
    {
        return tl::make_unexpected(make_error_code(JwtError::JwtFormatError));
    }
    catch (std::runtime_error &e) 
    {
        return tl::make_unexpected(make_error_code(JwtError::JwtDecodingError));
    }

    return EnTTidType{0}; // TODO: return user entity ID
    
}

} // namespace YLineServer::Jwt