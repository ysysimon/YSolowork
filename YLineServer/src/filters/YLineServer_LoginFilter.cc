/**
 *
 *  YLineServer_LoginFilter.cc
 *
 */

#include "YLineServer_LoginFilter.h"
#include "spdlog/spdlog.h"
#include <json/value.h>
#include <string>
#include "utils/jwt.h"

using namespace drogon;
using namespace YLineServer;

void LoginFilter::doFilter(const HttpRequestPtr &req,
                         FilterCallback &&fcb,
                         FilterChainCallback &&fccb)
{
    // 从请求头中获取Authorization字段
    const auto& authHeader = req->getHeader("Authorization");
    Json::Value payload;
    std::string err;
    if (authHeader.empty() || !Jwt::validateBearerToken(authHeader, payload, err))
    {
        //Check failed
        Json::Value json;
        json["error"] = "Authorization Failed 鉴权失败";
        auto res = drogon::HttpResponse::newHttpJsonResponse(json);
        res->setStatusCode(k401Unauthorized);
        fcb(res);
        spdlog::warn("{} - Authorization Failed 鉴权失败", req->getPeerAddr().toIpPort());
        if (!err.empty()) 
        {
            spdlog::error("JWT Error: {}", err);
        }
        return;
    }
    

    //Passed
    spdlog::debug("Payload: {}", payload.toStyledString());
    req->attributes()->insert("JWTpayload", payload);

    fccb();
}
