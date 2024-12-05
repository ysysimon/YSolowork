#include "YLineServer_WorkCtrl.h"

#include "json/value.h"
#include <algorithm>
#include <format>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>
#include <vector>
#include "utils/api.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/exception.hpp> // 包含异常定义

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
    if(!json.isMember("job_id"))
    {
        err = "JSON Error: Missing `job_id` field, 缺少 `job_id` 字段";
        return false;
    }

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
resolve_DAG
(
    std::vector<Components::Task> &task_Components,
    const std::unordered_map<std::string, std::size_t> &taskID2Index,
    const std::unordered_map<std::string, std::vector<std::string>> &Dependency,
    std::string &err
)
{
    using namespace boost;
    using Graph = boost::adjacency_list<vecS, vecS, directedS>;

    // build DAG
    Graph DAG(task_Components.size());
    for(const auto& [task_id, dependencies]: Dependency)
    {
        for(const auto& dependency: dependencies)
        {   
            // 添加依赖，表示 task_id 依赖于 dependency
            add_edge(taskID2Index.at(dependency), taskID2Index.at(task_id), DAG);
        }
    }

    // topological sort
    try
    {
        std::vector<Graph::vertex_descriptor> sorted;
        boost::topological_sort(DAG, std::back_inserter(sorted));
        std::reverse(sorted.begin(), sorted.end()); // reverse the order

        // print the sorted order
        std::size_t order = 0;
        for(auto v: sorted)
        {
            task_Components[v].order = order;
            order++;
            spdlog::info("task id: {}, order: {}, name: {}, belongJob_id: {}, dependency: {}, complete: {}", 
                task_Components[v].task_id, 
                task_Components[v].order, 
                task_Components[v].name, 
                task_Components[v].belongJob_id, 
                task_Components[v].dependency, 
                task_Components[v].complete
            );
        }
    }
    catch (const boost::not_a_dag &e)
    {
        err = "DAG Error: The task dependency resolved a cycle, 任务依赖关系形成了环";
        return false;
    }

    return true;
}

bool
WorkCtrl::resolveJob(const Json::Value &json, std::string &err, bool dependency)
{
    std::string job_id = json["job_id"].asString();
    std::vector<Components::Task> task_Components;

    // valid task dict is actually done here
    std::unordered_set<std::string> taskIDSet;
    for (std::size_t i = 0; i < json["tasks"].size(); i++) 
    {
        const auto &task = json["tasks"].get(i, Json::nullValue);

        if (!task.isMember("task_id"))
        {
            err = "JSON Error: Missing `task_id` field in task, 缺少 `task_id` 字段";
            return false;
        }

        if (!task["task_id"].isString())
        {
            err = "JSON Error: `task_id` field should be a string, `task_id` 字段应为字符串";
            return false;
        }

        if (!task.isMember("name") && task.isString())
        {
            err = "JSON Error: Missing `name` field in task, 缺少 `name` 字段";
            return false;
        }

        std::string task_id = task["task_id"].asString();
        if(dependency)
        {
            if(!task.isMember("dependency"))
            {
                err = "JSON Error: Missing `dependency` field in task, 缺少 `dependency` 字段";
                return false;
            }

            if(!task["dependency"].isArray())
            {
                err = "JSON Error: `dependency` field should be an array, `dependency` 字段应为数组";
                return false;
            }

            taskIDSet.insert(task_id);
        }
    }

    // need to use two-step to adaptive the dependency
    std::unordered_map<std::string, std::size_t> taskID2Index;
    std::unordered_map<std::string, std::vector<std::string>> Dependency;
    for (std::size_t i = 0; i < json["tasks"].size(); i++) 
    {
        const auto &task = json["tasks"].get(i, Json::nullValue);
        
        const std::string &id = task["task_id"].asString();
        const std::string &name = task["name"].asString();
        const std::string &belongJob_id = job_id;

        Components::Task taskCom{
            id, 
            i, // default order is submition order
            name, 
            belongJob_id, 
            false, // default no dependency
            false // default not complete
        };

        if(dependency)
        {
            if (!task["dependency"].empty())
            {
                taskCom.dependency = true;
            }
            
            for(const auto &dependency: task["dependency"])
            {
                if(!dependency.isString())
                {
                    err = "JSON Error: The dependent task id should be a string, 依赖任务 id 应为字符串";
                    return false;
                }
                    
                std::string dependencyId = dependency.asString();
                if(taskIDSet.find(dependencyId) == taskIDSet.end())
                {
                    err = std::format("JSON Error: Task {} dependent on {} but it does not exist, 任务 {} 依赖于 {} 但是它不存在", id, dependencyId, id, dependencyId); 
                    return false;
                }

                Dependency[id].push_back(dependencyId);
            }
        }

        // add task component
        taskID2Index[id] = i;
        task_Components.push_back(taskCom);
    }

    // debug print
    // for(auto &task_Component: task_Components)
    // {
    //     spdlog::info(
    //         "task id: {}, order: {}, name: {}, belongJob_id: {}, dependency: {}, complete: {}", 
    //         task_Component.task_id, 
    //         task_Component.order, 
    //         task_Component.name, 
    //         task_Component.belongJob_id, 
    //         task_Component.dependency, 
    //         task_Component.complete
    //     );
    // }

    // resolve DAG
    if (dependency)
    {
        if (!resolve_DAG(task_Components, taskID2Index, Dependency, err))
        {
            return false;
        }
    }
    
    return true;
}


drogon::Task<void> 
WorkCtrl::submitNonDependentJob(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback)
{
    // 解析请求体
    const auto& json = req->getJsonObject();
    if (!json) {
        // 返回错误信息
        callbackErrorJson(req, callback, "Invalid JSON 无效的 JSON");
        co_return;
    }

    std::string err;
    // 验证 JSON
    if (!this->validJobJson(*json, err)) 
    {
        callbackErrorJson(req, callback, err);
        co_return;
    }

    // 解析任务
    if (!resolveJob(*json, err)) 
    {
        callbackErrorJson(req, callback, err);
        co_return;
    }

    Json::Value respJson;
    respJson["message"] = "Job submitted 任务提交成功";
    auto resp = YLineServer::Api::makeJsonResponse(respJson, drogon::k200OK, req);
    callback(resp);

    co_return;

}


drogon::Task<void> 
WorkCtrl::submitDependentJob(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback)
{
    // 解析请求体
    const auto& json = req->getJsonObject();
    if (!json) {
        // 返回错误信息
        callbackErrorJson(req, callback, "Invalid JSON 无效的 JSON");
        co_return;
    }

    std::string err;
    // 验证 JSON
    if (!this->validJobJson(*json, err)) 
    {
        callbackErrorJson(req, callback, err);
        co_return;
    }

    // 解析任务
    if (!resolveJob(*json, err, true)) 
    {
        callbackErrorJson(req, callback, err);
        co_return;
    }

    Json::Value respJson;
    respJson["message"] = "Job submitted 任务提交成功";
    auto resp = YLineServer::Api::makeJsonResponse(respJson, drogon::k200OK, req);
    callback(resp);

    co_return;
}