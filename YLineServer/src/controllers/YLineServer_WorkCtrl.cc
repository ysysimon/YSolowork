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
    // job id needs to be gerenated by the database
    // if(!json.isMember("job_id"))
    // {
    //     err = "JSON Error: Missing `job_id` field, 缺少 `job_id` 字段";
    //     return false;
    // }

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
            // correct the order
            task_Components[v].order = order;
            order++;
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
WorkCtrl::resolveTasks(const Json::Value &json, std::string &err, std::vector<Components::Task> &task_Components, bool dependency)
{
    // job id needs to be gerenated by the database
    // const std::string &job_id = json["job_id"].asString();
    // std::vector<Components::Task> task_Components; // use pass in reference instead of return value

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

        // job id needs to be gerenated by the database
        // const std::string &belongJob_id = job_id;

        Components::Task taskCom{
            id, 
            static_cast<int>(i), // default order is submition order
            name, 
            // belongJob_id, // // use database generated job id
            false, // default no dependency
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

    // resolve DAG
    if (dependency)
    {
        if (!resolve_DAG(task_Components, taskID2Index, Dependency, err))
        {
            return false;
        }

        // sort the task components according to the order, which is the topological order
        std::sort
        (
            task_Components.begin(), task_Components.end(), 
            [](const Components::Task &a, const Components::Task &b) 
            {
                return a.order < b.order;
            }
        );
    }

    const std::string &job_name = json["job_name"].asString();
    const std::string &submit_user = json["submit_user"].asString();
    spdlog::debug("Task of Job - {} submitted by {} has resolved", job_name, submit_user);
    
    return true;
}

bool
WorkCtrl::resolveJob(const Json::Value &json, std::string &err, const HttpRequestPtr req, Components::Job &job_Component)
{
    // job id needs to be gerenated by the database
    // if(!json["job_id"].isString())
    // {
    //     err = "JSON Error: `job_id` field should be a string, `job_id` 字段应为字符串";
    //     return false;
    // }

    if(!json["jobName"].isString())
    {
        err = "JSON Error: `jobName` field should be a string, `jobName` 字段应为字符串";
        return false;
    }

    if(!req->attributes()->find("JWTpayload"))
    {
        err = "Missing JWTpayload in attributes, 在 attributes 中找不到 JWTpayload";
        return false;
    }

    // job id needs to be gerenated by the database
    // const std::string &job_id = json["job_id"].asString();
    const std::string &jobName = json["jobName"].asString();
    // 从 JWTpayload 中获取 username
    const auto& payload = req->attributes()->get<Json::Value>("JWTpayload");
    if (!payload.isMember("username") && !payload["username"].isString())
    {
        err = "Missing `username` field in JWT payload, JWT 载荷中缺少 `username` 字段";
        return false;
    
    }
    const std::string &submit_user = payload["username"].asString();

    job_Component = Components::Job{
        jobName,
        submit_user,
    };

    spdlog::debug("job - {} submitted by {} has resolved", jobName, submit_user);

    return true;
}

drogon::Task<void>
WorkCtrl::jobSubmit2DBTrans(const Components::Job &job_Component, const std::vector<Components::Task> &task_Components)
{
    try
    {
        auto dbClient = drogon::app().getFastDbClient("YLinedb");

        auto transPtr = co_await dbClient->newTransactionCoro();
        // 插入 job 并获取生成的 job_id
        auto result = co_await transPtr->execSqlCoro
        (
            "INSERT INTO job (job_name, submit_user) "
            "VALUES ($1, $2) RETURNING id",
            job_Component.name,
            job_Component.submit_user
        );

        if (result.empty())
        {
            transPtr->rollback();
            spdlog::error(
                "Job - {} submitted by {} failed to insert into database, return job_id is empty 任务提交失败, 返回的 job_id 为空", 
                job_Component.name, job_Component.submit_user
            );
            co_return;
        }

        // 获取生成的 job id
        int job_id = result[0]["id"].as<int>();
        // 插入 task
        for(const auto &task: task_Components)
        {
            co_await transPtr->execSqlCoro
            (
                "INSERT INTO task (task_id, job_id, task_name, task_order, dependency) "
                "VALUES ($1, $2, $3, $4, $5)",
                task.task_id,
                job_id,
                task.name,
                task.order,
                task.dependency
            );
        }

    }
    catch (const drogon::orm::DrogonDbException &e) 
    {
        // 自动回滚
        spdlog::error(
            "Job - {} submitted by {} failed to insert into database, {} 任务提交失败, 数据库异常", 
            job_Component.name, job_Component.submit_user, e.base().what()
        );
        co_return;
    }
    
    spdlog::info(
        "Job - {} submitted by {} is being inserted into database 任务提交成功, 已插入数据库", 
        job_Component.name, job_Component.submit_user
    );

    co_return;
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
    Components::Job job_Component;
    if(!resolveJob(*json, err, req, job_Component))
    {
        callbackErrorJson(req, callback, err);
        co_return;
    }

    std::vector<Components::Task> task_Components;
    if (!resolveTasks(*json, err, task_Components)) 
    {
        callbackErrorJson(req, callback, err);
        co_return;
    }

    co_await jobSubmit2DBTrans(job_Component, task_Components);

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
    Components::Job job_Component;
    if(!resolveJob(*json, err, req, job_Component))
    {
        callbackErrorJson(req, callback, err);
        co_return;
    }
    
    std::vector<Components::Task> task_Components;
    if (!resolveTasks(*json, err, task_Components, true)) 
    {
        callbackErrorJson(req, callback, err);
        co_return;
    }

    co_await jobSubmit2DBTrans(job_Component, task_Components);

    Json::Value respJson;
    respJson["message"] = "Job submitted 任务提交成功";
    auto resp = YLineServer::Api::makeJsonResponse(respJson, drogon::k200OK, req);
    callback(resp);

    co_return;
}