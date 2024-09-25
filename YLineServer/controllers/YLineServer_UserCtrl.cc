#include "YLineServer_UserCtrl.h"
#include "drogon/orm/CoroMapper.h"
#include "models/Users.h" // ORM Model

using namespace YLineServer;

using namespace drogon_model::yline;

using namespace drogon::orm;

// Add definition of your processing function here
drogon::Task<void> UserCtrl::getUserById(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback, int userId)
{
    // 获取数据库客户端
    auto dbClient = drogon::app().getDbClient("default");
    // 创建ORM Mapper
    drogon::orm::CoroMapper<Users> mapper(dbClient);

    // 查询数据库
    try {
        auto user = co_await mapper.findByPrimaryKey(userId);
        // 返回查询结果
        auto json = user.toJson();
        auto resp = HttpResponse::newHttpJsonResponse(json);
        callback(resp);
    } catch (const DrogonDbException &e) {
        // 返回错误信息
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k500InternalServerError);
        resp->setBody(e.base().what());
        callback(resp);
    }
    
    co_return;
}
