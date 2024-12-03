#include "YLineServer_WorkCtrl.h"

#include <spdlog/spdlog.h>
#include <string>
#include "utils/api.h"

using namespace YLineServer;

void 
callbackErrorJson(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback, const std::string &err)
{
    const auto& peerAddr = req->getPeerAddr();
    Json::Value respJson;
    respJson["error"] = err;
    auto resp = YLineServer::Api::makeJsonResponse(respJson, drogon::k400BadRequest, req);
    callback(resp);
    spdlog::error("{} WorkCtrl: {}", peerAddr.toIpPort(), err);
}

bool
WorkCtrl::validJobJson(const Json::Value &json, std::string &err)
{

    if(!json.isMember("jobName"))
    {
        err = "JSON Error: Missing `jobName` field, 缺少 `jobName` 字段";
        return false;
    }

    if(!json.isMember("tasks"))
    {
        err = "JSON Error: Missing `tasks` field, 缺少 `tasks` 字段";
        return false;
    }

    if(!json["tasks"].isArray())
    {
        err = "JSON Error: `tasks` field should be an array, `tasks` 字段应为数组";
        return false;
    }

    return true;
}

bool
resolveJob(const Json::Value &json, std::string &err, bool dependency = false)
{
    std::string jobName = json["jobName"].asString();

    std::unordered_set<std::string> taskSet;
    // valid task dict is actually done here
    for(const auto & task: json["tasks"])
    {
        if (!task.isMember("id") && task.isInt())
        {
            err = "JSON Error: Missing `id` field in task, 缺少 `id` 字段";
            return false;
        }

        if (!task.isMember("name") && task.isString())
        {
            err = "JSON Error: Missing `name` field in task, 缺少 `name` 字段";
            return false;
        }

        int id = task["id"].asInt();
        std::string name = task["name"].asString();
        std::string belongJob = jobName;

        taskSet.insert(name);
    }


    return true;
}


drogon::Task<void> WorkCtrl::submitNonDependentJob(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback)
{
    // 解析请求体
    const auto& json = req->getJsonObject();
    if (!json) {
        // 返回错误信息
        callbackErrorJson(req, callback, "Invalid JSON 无效的 JSON");
        co_return;
    }

    // 验证 JSON
    std::string err;
    if (!this->validJobJson(*json, err)) 
    {
        callbackErrorJson(req, callback, err);
        co_return;
    }

    // 解析任务


}
