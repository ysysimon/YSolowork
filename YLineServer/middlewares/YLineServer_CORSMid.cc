#include "YLineServer_CORSMid.h"
#include "spdlog/spdlog.h"

using namespace YLineServer;

CORSMiddleware::CORSMiddleware(const std::unordered_set<std::string>& allowedOrigins)
    : allowedOrigins_(allowedOrigins) 
    {
    }

// Task<HttpResponsePtr> CORSMiddleware::invoke(const HttpRequestPtr &req, MiddlewareNextAwaiter &&next)
// {
//     const std::string& origin = req->getHeader("Origin");
//     // 检查是否允许所有来源 (通配符 *)
//     bool allowAllOrigins = (allowedOrigins_.find("*") != allowedOrigins_.end());
//     spdlog::info("被调用！！！");

//     // 处理 OPTIONS 预检请求
//     if (req->method() == Options)
//     {
//         spdlog::info("CORSMiddleware invoked by {} 跨域中间件被调用", origin);
//         // 检查 Origin 是否被允许
//         if (!allowAllOrigins && allowedOrigins_.find(origin) == allowedOrigins_.end())
//         {
//             auto resp = HttpResponse::newHttpResponse();
//             resp->setStatusCode(k403Forbidden);
//             resp->setBody("403 Forbidden - Origin not allowed 非法跨域请求");
//             mcb(resp);  // 返回响应，终止进一步处理
//             spdlog::warn("Origin {} not allowed, 403 Forbidden 非法跨域请求", origin);
//             return;
//         }
//         // 返回预检请求响应
//         auto resp = HttpResponse::newHttpResponse();
//         resp->setStatusCode(k200OK);
//         if (allowAllOrigins)
//         {
//             resp->addHeader("Access-Control-Allow-Origin", "*");
//         }
//         else
//         {
//             resp->addHeader("Access-Control-Allow-Origin", origin);
//         }
//         resp->addHeader("Access-Control-Allow-Methods", "GET,POST,PUT,DELETE,OPTIONS");
//         resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
//         resp->addHeader("Access-Control-Allow-Credentials", "true");
//         mcb(resp);  // 返回响应，终止进一步处理
//         return;
//     }

//     // nextCb([ mcb = std::move(mcb)](const HttpResponsePtr& resp){ mcb(nullptr); });
    
//     // 继续处理普通请求
//     nextCb([origin, mcb = std::move(mcb), allowAllOrigins](const HttpResponsePtr& resp) mutable {
//         spdlog::info("普通请求");
//         // 在响应中添加 CORS 头
//         if (allowAllOrigins)
//         {
//             resp->addHeader("Access-Control-Allow-Origin", "*");
//         }
//         else
//         {
//             resp->addHeader("Access-Control-Allow-Origin", origin);
//         }
//         resp->addHeader("Access-Control-Allow-Credentials", "true");
//         mcb(resp);  // 返回最终的响应
//     });

// }

Task<HttpResponsePtr> CORSMiddleware::invoke(const HttpRequestPtr &req, MiddlewareNextAwaiter &&next)
{
    const std::string& origin = req->getHeader("Origin");
    // 检查是否允许所有来源 (通配符 *)
    bool allowAllOrigins = (allowedOrigins_.find("*") != allowedOrigins_.end());
    spdlog::info("被调用！！！");

    // 处理 OPTIONS 预检请求
    if (req->method() == Options)
    {
        spdlog::info("CORSMiddleware invoked by {} 跨域中间件被调用", origin);

        // 检查 Origin 是否被允许
        if (!allowAllOrigins && allowedOrigins_.find(origin) == allowedOrigins_.end())
        {
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k403Forbidden);
            resp->setBody("403 Forbidden - Origin not allowed 非法跨域请求");
            spdlog::warn("Origin {} not allowed, 403 Forbidden 非法跨域请求", origin);
            co_return resp;  // 返回响应，终止进一步处理
        }

        // 返回预检请求响应
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k200OK);
        if (allowAllOrigins)
        {
            resp->addHeader("Access-Control-Allow-Origin", "*");
        }
        else
        {
            resp->addHeader("Access-Control-Allow-Origin", origin);
        }
        resp->addHeader("Access-Control-Allow-Methods", "GET,POST,PUT,DELETE,OPTIONS");
        resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        resp->addHeader("Access-Control-Allow-Credentials", "true");

        co_return resp;  // 返回响应，终止进一步处理
    }

    // 继续处理普通请求，等待下一个中间件或控制器
    auto resp = co_await next;  // 继续请求处理

    spdlog::info("普通请求");

    // 在响应中添加 CORS 头
    if (allowAllOrigins)
    {
        resp->addHeader("Access-Control-Allow-Origin", "*");
    }
    else
    {
        resp->addHeader("Access-Control-Allow-Origin", origin);
    }
    resp->addHeader("Access-Control-Allow-Credentials", "true");

    co_return resp;  // 返回最终的响应
}