#include "YLineServer_UserCtrl.h"
#include "drogon/HttpTypes.h"
#include "drogon/orm/CoroMapper.h"
#include "drogon/orm/Criteria.h"
#include "models/Users.h" // ORM Model
#include <json/json.h>
#include <json/value.h>
#include <spdlog/spdlog.h>
#include <string>

#include "utils/passwd.h"
#include "utils/api.h"
#include "utils/jwt.h"

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

    const auto& peerAddr = req->getPeerAddr();
    // 查询数据库
    try {
        auto user = co_await mapper.findByPrimaryKey(userId);
        // 返回查询结果
        Json::Value userJson = user.toJson();
        userJson.removeMember("password");
        userJson.removeMember("salt");
        // auto resp = HttpResponse::newHttpJsonResponse(userJson);
        // resp->setStatusCode(drogon::k200OK);
        auto resp = YLineServer::Api::makeJsonResponse(userJson, drogon::k200OK, req);
        callback(resp);
        spdlog::info("{} Get User: User found 用户查询成功", peerAddr.toIpPort());
    } catch (const UnexpectedRows &e) {
        // 返回错误信息
        Json::Value respJson;
        respJson["error"] = "User not found";
        respJson["userId"] = userId;
        // auto resp = HttpResponse::newHttpJsonResponse(respJson);
        // resp->setStatusCode(drogon::k404NotFound);
        auto resp = YLineServer::Api::makeJsonResponse(respJson, drogon::k404NotFound, req);
        callback(resp);
        spdlog::info("{} Get User: User not found 用户不存在", peerAddr.toIpPort());
    } catch (const DrogonDbException &e) {
        // 返回错误信息
        Json::Value respJson;
        respJson["error"] = e.base().what();
        respJson["userId"] = userId;
        // auto resp = HttpResponse::newHttpJsonResponse(respJson);
        // resp->setStatusCode(drogon::k500InternalServerError);
        auto resp = YLineServer::Api::makeJsonResponse(respJson, drogon::k500InternalServerError, req);
        callback(resp);
        spdlog::error("{} Get User: {}", peerAddr.toIpPort(), e.base().what());
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
        // auto resp = HttpResponse::newHttpJsonResponse(respJson);
        // resp->setStatusCode(drogon::k400BadRequest);
        auto resp = YLineServer::Api::makeJsonResponse(respJson, drogon::k400BadRequest, req);
        callback(resp);
        spdlog::error("{} Create User: Invalid JSON 无效的 JSON", peerAddr.toIpPort());
        co_return;
    }
    
    std::string err;
    if (!Users::validateJsonForCreation(*json, err)) {
        // 返回错误信息
        Json::Value respJson;
        respJson["error"] = "Invalid User info 无效的用户信息";
        // auto resp = HttpResponse::newHttpJsonResponse(respJson);
        // resp->setStatusCode(drogon::k400BadRequest);
        auto resp = YLineServer::Api::makeJsonResponse(respJson, drogon::k400BadRequest, req);
        callback(resp);
        spdlog::error("{} Create User: Invalid User info 无效的用户信息 \n {}", peerAddr.toIpPort(), err);
        co_return;
    }

    // 创建用户
    Users user(*json);
    // 生成密码盐并加密密码
    const std::string& salt = Passwd::generateSalt();
    user.setPassword(Passwd::hashPassword(user.getValueOfPassword(), salt));
    user.setSalt(salt);

    // 插入新用户
    try {
        co_await mapper.insert(user);
        // 返回成功信息
        Json::Value respJson;
        respJson["message"] = "User created 用户创建成功";
        // auto resp = HttpResponse::newHttpJsonResponse(respJson);
        // resp->setStatusCode(drogon::k201Created);
        auto resp = YLineServer::Api::makeJsonResponse(respJson, drogon::k201Created, req);
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

drogon::Task<void> UserCtrl::login(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback)
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
        // auto resp = HttpResponse::newHttpJsonResponse(respJson);
        // resp->setStatusCode(drogon::k400BadRequest);
        auto resp = YLineServer::Api::makeJsonResponse(respJson, drogon::k400BadRequest, req);
        callback(resp);
        spdlog::error("{} Login: Invalid JSON 无效的 JSON", peerAddr.toIpPort());
        co_return;
    }

    // 检查请求体是否包含用户名和密码
    if (!json->isMember("username") || !json->isMember("password")) {
        // 返回错误信息
        Json::Value respJson;
        respJson["error"] = "Invalid User info 无效的用户信息";
        // auto resp = HttpResponse::newHttpJsonResponse(respJson);
        // resp->setStatusCode(drogon::k400BadRequest);
        auto resp = YLineServer::Api::makeJsonResponse(respJson, drogon::k400BadRequest, req);
        callback(resp);
        spdlog::error("{} Login: Invalid User info 无效的用户信息", peerAddr.toIpPort());
        co_return;
    }
    
    const auto& username = json->get("username", "").asString();
    const auto& inputPassword = json->get("password", "").asString();
    // 查询用户
    try {
        // 不要使用引用接收查询结果，很危险，会导致登陆一定成功
        // 常量引用 无法延长协程返回值的 生命周期 ！！！
        const Users user = co_await mapper.findOne(
            Criteria(Users::Cols::_username, 
            CompareOperator::EQ, 
            username)
            );
        // spdlog::info("username: {}", user.getValueOfUsername());
        // spdlog::info("DBpasswdHash: {}", user.getValueOfPassword());
        // spdlog::info("salt: {}", user.getValueOfSalt());
        const auto& DBpasswdHash = user.getValueOfPassword();
        const auto& inputpasswdHash = Passwd::hashPassword(inputPassword, user.getValueOfSalt());
        // spdlog::info("inputpasswdHash: {}", inputpasswdHash);
        // 检查密码
        if (Passwd::compareHash(DBpasswdHash,inputpasswdHash)) {
            // 返回成功信息
            // set user info
            Json::Value userJson = user.toJson();
            userJson.removeMember("password");
            userJson.removeMember("salt");

            // set jwt
            const std::string& jwt = Jwt::generateAuthJwt(user.getValueOfId(), user.getValueOfUsername(), user.getValueOfIsAdmin());

            Json::Value respJson;
            respJson["message"] = "Login success 登录成功";
            respJson["user"] = userJson;
            respJson["access_token"] = jwt;
            // auto resp = HttpResponse::newHttpJsonResponse(respJson);
            // resp->setStatusCode(drogon::k200OK);
            auto resp = YLineServer::Api::makeJsonResponse(respJson, drogon::k200OK, req);
            callback(resp);
            spdlog::info("{} Login as {}: Login success 登录成功", peerAddr.toIpPort(), username);
        } else {
            // 返回错误信息
            Json::Value respJson;
            respJson["error"] = "Invalid password 密码错误";
            // auto resp = HttpResponse::newHttpJsonResponse(respJson);
            // resp->setStatusCode(drogon::k401Unauthorized);
            auto resp = YLineServer::Api::makeJsonResponse(respJson, drogon::k401Unauthorized, req);
            callback(resp);
            spdlog::warn("{} Login as {}: Invalid password 密码错误", peerAddr.toIpPort(), username);
        }
    } catch (const UnexpectedRows &e) {
        // 返回错误信息
        Json::Value respJson;
        respJson["error"] = "User not found 用户不存在";
        // auto resp = HttpResponse::newHttpJsonResponse(respJson);
        // resp->setStatusCode(drogon::k404NotFound);
        auto resp = YLineServer::Api::makeJsonResponse(respJson, drogon::k404NotFound, req);
        callback(resp);
        spdlog::warn("{} Login as {}: User not found 用户不存在", peerAddr.toIpPort(), username);
    } catch (const DrogonDbException &e) {
        // 返回错误信息
        Json::Value respJson;
        respJson["error"] = e.base().what();
        // auto resp = HttpResponse::newHttpJsonResponse(respJson);
        // resp->setStatusCode(drogon::k500InternalServerError);
        auto resp = YLineServer::Api::makeJsonResponse(respJson, drogon::k500InternalServerError, req);
        callback(resp);
        spdlog::error("{} Login as {}: {}", peerAddr.toIpPort(), username, e.base().what());
    }

    co_return;
}

// void UserCtrl::login(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr &)>&& callback)
// {
//     // 获取数据库客户端, 数据库使用 fast 模式 需要使用 fastDbClient
//     auto dbClient = drogon::app().getFastDbClient("YLinedb");
    
//     // 解析请求体
//     const auto& json = req->getJsonObject();
//     const auto& peerAddr = req->getPeerAddr();
//     if (!json) {
//         // 返回错误信息
//         Json::Value respJson;
//         respJson["error"] = "Invalid JSON 无效的 JSON";
//         auto resp = HttpResponse::newHttpJsonResponse(respJson);
//         resp->setStatusCode(drogon::k400BadRequest);
//         callback(resp);
//         spdlog::error("{} Login: Invalid JSON 无效的 JSON", peerAddr.toIpPort());
//         return;
//     }

//     // 检查请求体是否包含用户名和密码
//     if (!json->isMember("username") || !json->isMember("password")) {
//         // 返回错误信息
//         Json::Value respJson;
//         respJson["error"] = "Invalid User info 无效的用户信息";
//         auto resp = HttpResponse::newHttpJsonResponse(respJson);
//         resp->setStatusCode(drogon::k400BadRequest);
//         callback(resp);
//         spdlog::error("{} Login: Invalid User info 无效的用户信息", peerAddr.toIpPort());
//         return;
//     }

//     const auto& username = json->get("username", "").asString();
//     const auto& inputPassword = json->get("password", "").asString();
    
//     // 查询用户
//     auto mapper = drogon::orm::Mapper<Users>(dbClient);
//     mapper.findOne(Criteria(Users::Cols::_username, CompareOperator::EQ, username),
//                    [=](const Users &user) {
//                        // 查询成功，开始处理密码验证
//                        const auto& DBpasswdHash = user.getValueOfPassword();
//                        const auto& inputpasswdHash = Passwd::hashPassword(inputPassword, user.getValueOfSalt());

//                        if (Passwd::compareHash(DBpasswdHash, inputpasswdHash)) {
//                            // 密码正确，返回成功信息
//                            Json::Value respJson;
//                            respJson["message"] = "Login success 登录成功";
//                            auto resp = HttpResponse::newHttpJsonResponse(respJson);
//                            resp->setStatusCode(drogon::k200OK);
//                            callback(resp);
//                            spdlog::info("{} Login as {}: Login success 登录成功", peerAddr.toIpPort(), username);
//                        } else {
//                            // 密码错误
//                            Json::Value respJson;
//                            respJson["error"] = "Invalid password 密码错误";
//                            auto resp = HttpResponse::newHttpJsonResponse(respJson);
//                            resp->setStatusCode(drogon::k401Unauthorized);
//                            callback(resp);
//                            spdlog::warn("{} Login as {}: Invalid password 密码错误", peerAddr.toIpPort(), username);
//                        }
//                    },
//                    [=](const DrogonDbException &e) {
//                        // 查询用户失败
//                        if (typeid(e) == typeid(UnexpectedRows)) {
//                            // 用户不存在
//                            Json::Value respJson;
//                            respJson["error"] = "User not found 用户不存在";
//                            auto resp = HttpResponse::newHttpJsonResponse(respJson);
//                            resp->setStatusCode(drogon::k404NotFound);
//                            callback(resp);
//                            spdlog::warn("{} Login as {}: User not found 用户不存在", peerAddr.toIpPort(), username);
//                        } else {
//                            // 数据库错误
//                            Json::Value respJson;
//                            respJson["error"] = e.base().what();
//                            auto resp = HttpResponse::newHttpJsonResponse(respJson);
//                            resp->setStatusCode(drogon::k500InternalServerError);
//                            callback(resp);
//                            spdlog::error("{} Login as {}: {}", peerAddr.toIpPort(), username, e.base().what());
//                        }
//                    });
// }
