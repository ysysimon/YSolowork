#include "jwt.h"

#include <json/value.h>
#include "jwt-cpp/traits/open-source-parsers-jsoncpp/traits.h"
#include <json/json.h>

namespace YLineServer::Jwt {

using traits = jwt::traits::open_source_parsers_jsoncpp;
using claim = jwt::basic_claim<traits>;


std::string generateAuthJwt(const Users::PrimaryKeyType& userId, const std::string& username, const bool isAdmin)
{
    const auto token = jwt::create<traits>()
        .set_issuer("YLineServer")
        .set_type("JWT")
        .set_payload_claim("userId", userId)
        .set_payload_claim("username", username)
        .set_payload_claim("isAdmin", isAdmin)
        .sign(jwt::algorithm::hs256{ "secret" });
        
    return token;
}


} // namespace YLineServer::Jwt