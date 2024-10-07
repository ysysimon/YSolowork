#include "utils/api.h"
#include "utils/config.h"

namespace YLineServer::Api {

void addCORSHeader(const HttpResponsePtr& resp, const HttpRequestPtr& req)
{
    auto& config = ConfigSingleton::getInstance().getConfigData();
    if (config.cors)
    {
        if (config.allowed_origins.find("*") != config.allowed_origins.end())
        {
            resp->addHeader("Access-Control-Allow-Origin", "*");
        }
        else
        {
            const std::string& origin = req->getHeader("Origin");
            if (config.allowed_origins.find(origin) != config.allowed_origins.end())
            {
                resp->addHeader("Access-Control-Allow-Origin", origin);
            }
        }
        
        resp->addHeader("Access-Control-Allow-Methods", "GET,POST,PUT,DELETE,OPTIONS");
        resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        resp->addHeader("Access-Control-Allow-Credentials", "true");
    }
}

HttpResponsePtr makeJsonResponse(const Json::Value& json, const HttpStatusCode& status, const HttpRequestPtr& req)
{
    auto resp = HttpResponse::newHttpJsonResponse(json);
    resp->setStatusCode(status);

    // 添加 CORS 头 这是由于 drogon 的中间件 似乎没有正确处理 Responds 的 CORS 头 先暂时在这里添加
    addCORSHeader(resp, req);

    return resp;

} 


}