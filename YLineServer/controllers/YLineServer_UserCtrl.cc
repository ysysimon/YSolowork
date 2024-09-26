#include "YLineServer_UserCtrl.h"
#include "drogon/HttpTypes.h"
#include "drogon/orm/CoroMapper.h"
#include "models/Users.h" // ORM Model
#include "json/value.h"
#include <json/json.h>
#include <spdlog/spdlog.h>
#include <string>

#include "utils/passwd.h"

using namespace YLineServer;

using namespace drogon_model::yline;

using namespace drogon::orm;

// Add definition of your processing function here
drogon::Task<void> UserCtrl::getUserById(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback, int userId)
{
    // 获取数据库客户端, 数据库使用 fast 模式 需要使用 fastDbClient
    auto dbClient = drogon::app().getFastDbClient("YLinedb");
    // 创建ORM Mapper
    drogon::orm::CoroMapper<Users> mapper(dbClient);

    // 查询数据库
    try {
        auto user = co_await mapper.findByPrimaryKey(userId);
        // 返回查询结果
        auto resp = HttpResponse::newHttpJsonResponse(user.toJson());
        resp->setStatusCode(drogon::k200OK);
        callback(resp);
    } catch (const UnexpectedRows &e) {
        // 返回错误信息
        Json::Value respJson;
        respJson["error"] = "User not found";
        respJson["userId"] = userId;
        auto resp = HttpResponse::newHttpJsonResponse(respJson);
        resp->setStatusCode(drogon::k404NotFound);
        callback(resp);
    } catch (const DrogonDbException &e) {
        // 返回错误信息
        Json::Value respJson;
        respJson["error"] = e.base().what();
        respJson["userId"] = userId;
        auto resp = HttpResponse::newHttpJsonResponse(respJson);
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
    
    co_return;
}

drogon::Task<void> UserCtrl::createUser(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback)
{
    // 获取数据库客户端, 数据库使用 fast 模式 需要使用 fastDbClient
    auto dbClient = drogon::app().getFastDbClient("YLinedb");
    // 创建ORM Mapper
    drogon::orm::CoroMapper<Users> mapper(dbClient);

    // 解析请求体
    const auto& json = req->getJsonObject();
    const auto& peerAddr = req->getPeerAddr();
    if (!json) {
        // 返回错误信息
        Json::Value respJson;
        respJson["error"] = "Invalid JSON 无效的 JSON";
        auto resp = HttpResponse::newHttpJsonResponse(respJson);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        spdlog::error("{} Create User: Invalid JSON 无效的 JSON", peerAddr.toIpPort());
        co_return;
    }
    
    std::string err;
    if (!Users::validateJsonForCreation(*json, err)) {
        // 返回错误信息
        Json::Value respJson;
        respJson["error"] = "Invalid User info 无效的用户信息";
        auto resp = HttpResponse::newHttpJsonResponse(respJson);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        spdlog::error("{} Create User: Invalid User info 无效的用户信息 \n {}", peerAddr.toIpPort(), err);
        co_return;
    }

    // 创建用户
    Users user(*json);
    // 生成密码盐并加密密码
    const std::string& salt = Passwd::generateSalt();
    user.setPasswordHash(Passwd::hashPassword(*user.getPasswordHash(), salt));
    user.setSalt(salt);

    // 保存用户
    try {
        co_await mapper.insert(user);
        // 返回成功信息
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k201Created);
        callback(resp);
        spdlog::info("{} Create User: User created 用户创建成功", peerAddr.toIpPort());
    } catch (const DrogonDbException &e) {
        // 返回错误信息
        Json::Value respJson;
        respJson["error"] = e.base().what();
        auto resp = HttpResponse::newHttpJsonResponse(respJson);
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }

    co_return;
}