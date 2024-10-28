/**
 *
 *  YLineServer_LoginFilter.cc
 *
 */

#include "YLineServer_LoginFilter.h"
#include "spdlog/spdlog.h"
#include <json/value.h>

using namespace drogon;
using namespace YLineServer;

void LoginFilter::doFilter(const HttpRequestPtr &req,
                         FilterCallback &&fcb,
                         FilterChainCallback &&fccb)
{
    // 从请求头中获取Authorization字段
    const auto& authHeader = req->getHeader("Authorization");
    Json::Value payload;
    if (authHeader.empty() || !validateToken(authHeader, payload))
    {
        //Check failed
        Json::Value json;
        json["error"] = "Authorization Failed 鉴权失败";
        auto res = drogon::HttpResponse::newHttpJsonResponse(json);
        res->setStatusCode(k401Unauthorized);
        fcb(res);
        return;
    }
    

    //Passed
    spdlog::debug("Payload: {}", payload.toStyledString());
    // spdlog::info("body: {}", req->body());
    req->setBody(std::move(payload.toStyledString()));
    // spdlog::info("body: {}", req->body());
    fccb();
}


bool LoginFilter::validateToken(const std::string &authHeader, Json::Value& payload)
{
    // 从Authorization头中提取JWT
    if (authHeader.find("Bearer ") != 0)
        return false;

    const std::string& token = authHeader.substr(7);  // 去掉"Bearer "部分

    // 验证 JWT
    std::string err;
    bool valid = Jwt::decodeAuthJwt(token, payload, err);

    if (!valid)
    {
        spdlog::error("JWT validation failed: {}", err);
    }

    return valid;
}