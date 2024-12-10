#include "drogon/HttpRequest.h"
#include "drogon/HttpTypes.h"
#include "drogon/HttpResponse.h"
#include "drogon/lib/src/impl_forwards.h"
#include "json/value.h"
#include <json/json.h>
#include "drogon/WebSocketConnection.h"

#ifndef YLINESERVER_API_H
#define YLINESERVER_API_H

using namespace drogon;

namespace YLineServer::Api {

// 函数: 添加 CORS 头
void addCORSHeader(const HttpResponsePtr& resp, const HttpRequestPtr& req);

// 函数: 生成 JSON 响应
HttpResponsePtr makeJsonResponse(const Json::Value& json, const HttpStatusCode& status, const HttpRequestPtr& req);

// 函数: 生成 HTML 响应
HttpResponsePtr makeHttpResponse(const std::string& body, const HttpStatusCode& status, const HttpRequestPtr& req, const ContentType& contentType);

// 函数: 解析 JSON
bool parseJson(const std::string& jsonStr, Json::Value& resultJson, std::string& errs);

//函数: 鉴权 WebSocket 连接
void authWebSocketConnection(const WebSocketConnectionPtr& wsConnPtr, const std::string& token, const std::string &from);

} // namespace YLineServer::Api
#endif // YLINESERVER_API_H