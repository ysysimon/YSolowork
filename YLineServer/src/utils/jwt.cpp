#include "utils/jwt.h"
#include "utils/config.h"

#include <json/value.h>
#include "jwt-cpp/traits/open-source-parsers-jsoncpp/traits.h"
#include <json/json.h>

namespace YLineServer::Jwt {

using traits = jwt::traits::open_source_parsers_jsoncpp;
using claim = jwt::basic_claim<traits>;


std::string generateAuthJwt(const Users::PrimaryKeyType& userId, const std::string& username, const bool isAdmin)
{
    const ConfigSingleton& config = ConfigSingleton::getInstance();
    const std::string& jwtSecret = config.getConfigData().jwt_secret;
    const auto token = jwt::create<traits>()
        .set_issuer("YLineServer")
        .set_type("JWT")
        .set_payload_claim("userId", userId)
        .set_payload_claim("username", username)
        .set_payload_claim("isAdmin", isAdmin)
        // no need to send secret to client, so we can use HS256, and it's good for Hexpansion
        .sign(jwt::algorithm::hs256{ jwtSecret });
        
    return token;
}


} // namespace YLineServer::Jwt