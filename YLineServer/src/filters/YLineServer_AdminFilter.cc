/**
 *
 *  YLineServer_AdminFilter.cc
 *
 */

#include "YLineServer_AdminFilter.h"
#include "spdlog/spdlog.h"

using namespace drogon;
using namespace YLineServer;

void faildRespond(const HttpRequestPtr &req, const std::string &msg, FilterCallback &&fcb)
{
    Json::Value json;
    json["error"] = msg;
    auto res = drogon::HttpResponse::newHttpJsonResponse(json);
    res->setStatusCode(k401Unauthorized);
    fcb(res);
    spdlog::warn("{} - Admin Filter: {}", req->getPeerAddr().toIpPort(), msg);
}

void AdminFilter::doFilter(const HttpRequestPtr &req,
                         FilterCallback &&fcb,
                         FilterChainCallback &&fccb)
{
    // 从 attributes 中获取 JWTpayload
    if(!req->attributes()->find("JWTpayload"))
    {
        faildRespond(req, "Missing JWTpayload in attributes, 在 attributes 中找不到 JWTpayload", std::move(fcb));
        return;
    }

    // 从 JWTpayload 中获取 role
    const auto& payload = req->attributes()->get<Json::Value>("JWTpayload");
    // spdlog::debug("Admin Filter: Payload: {}", payload.toStyledString());

    if (!payload.isMember("isAdmin") && !payload["isAdmin"].isBool())
    {
        faildRespond(req, "Missing `isAdmin` field in JWT payload, JWT 载荷中缺少 `isAdmin` 字段", std::move(fcb));
        return;
    }

    // Warning: this not garantee the role state is up-to-date with database
    if (!payload["isAdmin"].asBool())
    {
        faildRespond(req, "Require Admin Role, 需要管理员权限", std::move(fcb));
        return;
    }

    // Passed
    fccb();
}
