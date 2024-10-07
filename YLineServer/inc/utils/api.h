#include "drogon/HttpRequest.h"
#include "drogon/HttpTypes.h"
#include "drogon/HttpResponse.h"
#include <json/json.h>

#ifndef YLINESERVER_API_H
#define YLINESERVER_API_H

using namespace drogon;

namespace YLineServer::Api {

// 函数: 添加 CORS 头
void addCORSHeader(const HttpResponsePtr& resp, const HttpRequestPtr& req);

// 函数: 生成 JSON 响应
HttpResponsePtr makeJsonResponse(const Json::Value& json, const HttpStatusCode& status, const HttpRequestPtr& req);


} // namespace YLineServer::Api
#endif // YLINESERVER_API_H