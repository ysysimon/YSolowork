#include "YLineServer_JobCtrl.h"

#include <cstdint>
#include "drogon/orm/CoroMapper.h"
#include <spdlog/spdlog.h>
#include <vector>
#include "utils/api.h"
#include "utils/server.h"
#include "models/Jobs.h"
#include "models/Tasks.h"
#include "job.h"

using namespace YLineServer;
using namespace drogon::orm;
using namespace drogon_model::yline;

void
failedResp(
    const HttpRequestPtr req, 
    const std::string &who,
    const std::string &what,
    std::function<void(const HttpResponsePtr &)> callback
)
{
    const auto &peerAddr = req->getPeerAddr();
    Json::Value respJson;
    respJson["error"] = what;
    auto resp = YLineServer::Api::makeJsonResponse(respJson, drogon::k400BadRequest, req);
    callback(resp);
    spdlog::warn(
        "JobCtrl has failed Respond from {} invoke {} 请求失败 : {} ",
        peerAddr.toIpPort(),
        who,
        what
    );
}

void
unlockJobatRedis(const int64_t jobId, const std::string &server_instance_uuid, const std::string &reason)
{
    auto redis = drogon::app().getFastRedisClient("YLineRedis");

    try
    {
        redis->execCommandAsync(
            [reason](const drogon::nosql::RedisResult &r) 
            {
                spdlog::debug("Because of `{}`, DEL JobLock result: {}", reason, r.asString());
            },
            [jobId, server_instance_uuid, reason](const std::exception &err)
            {
                spdlog::error(
                    "Job - {} unlock failed, server instance {} try to unlock it because of `{}` but failed , error: {} 解锁任务失败",
                    jobId,
                    server_instance_uuid,
                    reason,
                    err.what()
                );
            },
            "DEL JobLock:%d", jobId 
        );
    }
    catch (const drogon::orm::DrogonDbException &e)
    {
        spdlog::error("{} try to Unlock Job failed - Redis Error 异常: {}", reason, e.base().what());
    }
}

drogon::Task<void> 
JobCtrl::queueJob(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback)
{   
    // 验证 json
    const auto json = req->getJsonObject();
    if(!json)
    {
        failedResp(req, "queueJob", "Invalid JSON 无效的 JSON", callback);
        co_return;
    }

    if(!json->isMember("job_id"))
    {
        failedResp(req, "queueJob", "Invalid JSON: missing `job_id` failed 缺少 `job_id` 字段", callback);
        co_return;
    }

    if (!(*json)["job_id"].isInt64())
    {
        failedResp(req, "queueJob", "Invalid JSON: `job_id` must be an integer `job_id` 类型错误", callback);
        co_return;
    }
    int64_t jobId = (*json)["job_id"].asInt64();

    // 验证身份
    const auto &payload = req->attributes()->get<Json::Value>("JWTpayload");
    if (!payload.isMember("username") && !payload["username"].isString()) 
    {
        failedResp(req, "queueJob", "Missing `username` field in JWT payload, JWT 载荷中缺少 `username` 字段", callback);
        co_return;
    }
    const std::string &submit_user = payload["username"].asString();

    // need to acquire job lock from redis first, thanks to coroutines we can avoid callback hell
    const std::string &server_instance_uuid = boost::uuids::to_string(ServerSingleton::getInstance().getServerInstanceUUID());
    auto redis = drogon::app().getFastRedisClient("YLineRedis");
    auto dbClient = drogon::app().getFastDbClient("YLinedb");

    // first check if the job exists in the database
    drogon::orm::CoroMapper<Jobs> jobMapper(dbClient);
    Jobs job;
    try 
    {
        job = co_await jobMapper.findByPrimaryKey(jobId);
    } 
    catch (const drogon::orm::UnexpectedRows &e) 
    {
        failedResp(req, "queueJob", "Job not found 任务未找到", callback);
        co_return;
    }
    catch (const drogon::orm::DrogonDbException &e) 
    {
        failedResp(req, "queueJob", std::format("Find Job - Database Error 数据库异常: {}", e.base().what()), callback);
        co_return;   
    }
    
    
    // acquired job lock from redis
    try
    {
        // note %d not $d, this string format, not sql format
        const auto redis_lock_Result = co_await redis->execCommandCoro(
            "SET JobLock:%d %s NX",
            jobId,
            server_instance_uuid.c_str() // need to convert to c string
        );

        if (redis_lock_Result.isNil()) 
        {
            failedResp(req, "queueJob", "Job is being processed by another server instance 任务正在被其他服务器实例处理中", callback);
            co_return;
        }
    }
    catch (const drogon::orm::DrogonDbException &e)
    {
        failedResp(req, "queueJob", std::format("Acquired Job Lock - Redis Error 异常: {}", e.base().what()), callback);
        co_return;
    }

    // ------------------ below here, when error occurs, we need to unlock the job ------------------

    // job lock acquired, now we can query job's tasks from database
    drogon::orm::CoroMapper<Tasks> taskMapper(dbClient);
    std::vector<Tasks> tasks;
    try
    {
        tasks = co_await taskMapper.orderBy(
            Tasks::Cols::_task_order, 
            SortOrder::ASC
        )
        .findBy(
            Criteria(
                Tasks::Cols::_job_id,  
                CompareOperator::EQ, 
                // jobId // this will cause error, it's needs to be int.....
                static_cast<int>(jobId)
            )
        );

        for(const auto &task : tasks)
        {
            spdlog::info(
                "Task - {} {} {} {} {} {}",
                *task.getTaskId(),
                *task.getTaskName(),
                *task.getTaskOrder(),
                *task.getJobId(),
                *task.getDependency(),
                *task.getStatus()
            );
        }
    }
    catch (const drogon::orm::DrogonDbException &e) 
    {
        failedResp(req, "queueJob", std::format("Find Tasks - Database Error 数据库异常: {}", e.base().what()), callback);
        // 'co_await' cannot be used in the handler of a try block, so we need to use callback to unlock the job
        unlockJobatRedis(jobId, server_instance_uuid, "Find Tasks - Database Error 数据库异常");
        co_return;   
    }

    // we get job and it's tasks, now we can queue the job
    

    spdlog::info("Job - {} request execute from {} has being queued 任务请求执行成功, 已进入队列", jobId, submit_user);
    co_return;
}