#include "utils/api.h"
#include "utils/server.h"
#include "utils/jwt.h"

namespace YLineServer::Api {

// this is a workaround maybe will be removed in the future
void addCORSHeader(const HttpResponsePtr& resp, const HttpRequestPtr& req)
{
    auto& config = ServerSingleton::getInstance().getConfigData();
    if (config.cors)
    {
        if (config.allowed_origins.find("*") != config.allowed_origins.end() || req == nullptr)
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

HttpResponsePtr makeHttpResponse(const std::string& body, const HttpStatusCode& status, const HttpRequestPtr& req, const ContentType& contentType)
{
    auto resp = HttpResponse::newHttpResponse();
    resp->setBody(body);
    resp->setStatusCode(status);
    resp->setContentTypeCode(contentType);

    // 添加 CORS 头 这是由于 drogon 的中间件 似乎没有正确处理 Responds 的 CORS 头 先暂时在这里添加
    addCORSHeader(resp, req);

    return resp;
}

bool parseJson(const std::string& jsonStr, Json::Value& resultJson, std::string& errs)
{
    Json::CharReaderBuilder reader;
    std::istringstream s(jsonStr);
    bool parse_result = Json::parseFromStream(reader, s, &resultJson, &errs);
    return parse_result;
}


void authWebSocketConnection(const WebSocketConnectionPtr& wsConnPtr, const std::string& token, const std::string &from)
{
    const auto& user = Jwt::verifyToken2EnTTUser(token);
    user.or_else(
        [wsConnPtr, &from](const std::error_code& err) 
        {
            spdlog::error(
                "{} - Failed to verify token: {} at {}", 
                wsConnPtr->peerAddr().toIpPort(), err.message(), from
            );
        }
    ).map(
        [wsConnPtr, &from](const EnTTidType& userId) 
        {
            wsConnPtr->setContext(std::make_shared<EnTTidType>(userId));
            const auto& username = ServerSingleton::getInstance().Registry.get<Components::User>(userId).username;
            spdlog::info(
                "{} - Authenticated as user: {} at {}", 
                wsConnPtr->peerAddr().toIpPort(), 
                username,
                from
            );
        }
    );
    
}

} // namespace YLineServer::Api