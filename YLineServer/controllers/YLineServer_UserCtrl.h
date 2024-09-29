#pragma once

#include <drogon/HttpController.h>

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
    ADD_METHOD_TO(UserCtrl::getUserById, "/api/user/{1}", Get);
    ADD_METHOD_TO(UserCtrl::createUser, "/api/user", Post);
    ADD_METHOD_TO(UserCtrl::login, "/api/auth/login", Post);

    METHOD_LIST_END
    // your declaration of processing function maybe like this:
    drogon::Task<void> getUserById(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback, int userId);
    drogon::Task<void> createUser(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback);
    drogon::Task<void> login(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback);
};
}
