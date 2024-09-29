#include "YLineServer_CORSMid.h"

using namespace YLineServer;

CORSMiddleware::CORSMiddleware(const std::unordered_set<std::string>& allowedOrigins)
    : allowedOrigins_(allowedOrigins) 
    {
    }

drogon::Task<drogon::HttpResponsePtr> CORSMiddleware::invoke(const drogon::HttpRequestPtr& req,
                                                             drogon::MiddlewareNextAwaiter&& next)
{
    const std::string &origin = req->getHeader("Origin");

    // 使用哈希集合检查 Origin 是否被允许
    if (allowedOrigins_.find(origin) == allowedOrigins_.end())
    {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k403Forbidden);
        resp->setBody("403 Forbidden - Origin not allowed 非法跨域请求");
        co_return resp;  // 返回 403 Forbidden 响应
    }

    // 处理 OPTIONS 预检请求
    if (req->method() == drogon::Options)
    {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k200OK);

        resp->addHeader("Access-Control-Allow-Origin", origin);
        resp->addHeader("Access-Control-Allow-Methods", "GET,POST,PUT,DELETE,OPTIONS");
        resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        resp->addHeader("Access-Control-Allow-Credentials", "true");

        co_return resp;
    }

    // 继续处理普通请求
    auto resp = co_await next;
    resp->addHeader("Access-Control-Allow-Origin", origin);
    resp->addHeader("Access-Control-Allow-Credentials", "true");
    co_return resp;
}