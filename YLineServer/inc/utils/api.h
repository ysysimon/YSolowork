#include "drogon/HttpRequest.h"
#include "drogon/HttpTypes.h"
#include "drogon/HttpResponse.h"
#include "drogon/lib/src/impl_forwards.h"
#include <json/json.h>

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
Json::Value parseJson(const std::string& jsonStr, std::string& errs);

} // namespace YLineServer::Api
#endif // YLINESERVER_API_H