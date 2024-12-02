#pragma once

#include <drogon/HttpMiddleware.h>
#include <unordered_set>

namespace YLineServer
{

using namespace drogon;

// Coroutine version
// class CORSMiddleware : public drogon::HttpCoroMiddleware<CORSMiddleware>
// {
// public:
//     static constexpr bool isAutoCreation = false;  // 明确设置为 false
//     CORSMiddleware(const std::unordered_set<std::string>& allowedOrigins);

//     Task<HttpResponsePtr> invoke(const HttpRequestPtr &req,
//                                 MiddlewareNextAwaiter &&next) override;

// private:
//     std::unordered_set<std::string> allowedOrigins_;
// };

// Callback version
class CORSMiddleware : public drogon::HttpMiddleware<CORSMiddleware>
{
public:
    static constexpr bool isAutoCreation = false;  // 明确设置为 false
    CORSMiddleware(const std::unordered_set<std::string>& allowedOrigins);

    void invoke(const HttpRequestPtr &req,
                        MiddlewareNextCallback &&nextCb,
                        MiddlewareCallback &&mcb) override;

private:
    std::unordered_set<std::string> allowedOrigins_;
};

}

