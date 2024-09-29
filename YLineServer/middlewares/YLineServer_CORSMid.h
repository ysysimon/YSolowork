#pragma once

#include <drogon/HttpMiddleware.h>
#include <unordered_set>

namespace YLineServer
{


class CORSMiddleware : public drogon::HttpCoroMiddleware<CORSMiddleware>
{
public:
    CORSMiddleware(const std::unordered_set<std::string>& allowedOrigins);

    // 协程版本的 invoke 方法
    drogon::Task<drogon::HttpResponsePtr> invoke(const drogon::HttpRequestPtr& req,
                                                 drogon::MiddlewareNextAwaiter&& next) override;

private:
    std::unordered_set<std::string> allowedOrigins_;
};

}

