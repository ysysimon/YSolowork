#pragma once

#include <drogon/HttpMiddleware.h>
#include <unordered_set>

namespace YLineServer
{

using namespace drogon;

class CORSMiddleware : public drogon::HttpCoroMiddleware<CORSMiddleware>
{
public:
    static constexpr bool isAutoCreation = false;  // 明确设置为 false
    CORSMiddleware(const std::unordered_set<std::string>& allowedOrigins);

    Task<HttpResponsePtr> invoke(const HttpRequestPtr &req,
                                MiddlewareNextAwaiter &&next) override;

private:
    std::unordered_set<std::string> allowedOrigins_;
};

}

