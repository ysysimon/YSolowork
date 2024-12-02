#pragma once

#include <drogon/HttpController.h>

#include "drogon/HttpTypes.h"
#include "drogon/utils/coroutine.h"
#include <drogon/orm/CoroMapper.h>            // ORM Mapper 用于数据库操作
#include <drogon/HttpAppFramework.h>      // 获取数据库客户端和其他框架功能
#include <trantor/utils/Date.h>           // 时间处理

using namespace drogon;

namespace YLineServer
{
class UserCtrl : public drogon::HttpController<UserCtrl>
{
  public:
    METHOD_LIST_BEGIN
    // use METHOD_ADD to add your custom processing function here;
    ADD_METHOD_TO(UserCtrl::getUserById, "/api/user/{1}", Get, "YLineServer::LoginFilter");
    ADD_METHOD_TO(UserCtrl::getAllUsers, "/api/users", Get, "YLineServer::LoginFilter");
    ADD_METHOD_TO(UserCtrl::createUser, "/api/user", Post, "YLineServer::LoginFilter");
    ADD_METHOD_TO(UserCtrl::updateUser, "/api/user/{1}", Put, "YLineServer::LoginFilter");
    ADD_METHOD_TO(UserCtrl::login, "/api/auth/login", Post);
    ADD_METHOD_TO(UserCtrl::logout, "/api/auth/logout", Post, "YLineServer::LoginFilter");

    METHOD_LIST_END
    // your declaration of processing function maybe like this:
    drogon::Task<void> getUserById(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback, int userId);
    drogon::Task<void> getAllUsers(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback);
    drogon::Task<void> createUser(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback);
    drogon::Task<void> updateUser(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback, int userId);
    drogon::Task<void> login(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback);
    drogon::Task<void> logout(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback);
    
};
}
