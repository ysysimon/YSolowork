#include "jwt.h"

#include <jwt-cpp/jwt.h>

namespace YLineServer::Jwt {



std::string generateAuthJwt(const std::string& userId, const std::string& username, const bool isAdmin)
{
    auto token = jwt::create()
        .set_issuer("YLineServer")
        .set_type("JWT")
        .set_payload_claim("userId", jwt::claim(userId))
        .set_payload_claim("username", jwt::claim(username))
        .set_payload_claim("isAdmin", jwt::claim(isAdmin))
        .sign(jwt::algorithm::hs256{ "secret" });
}


} // namespace YLineServer::Jwt