#include "utils/jwt.h"
#include "utils/server.h"

#include <json/value.h>
#include "jwt-cpp/traits/open-source-parsers-jsoncpp/traits.h"
#include <json/json.h>

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


} // namespace YLineServer::Jwt