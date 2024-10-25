#include "YLineServer_WorkerCtrl.h"

#include "entt/entity/fwd.hpp"
#include "models/Workers.h"
#include "spdlog/spdlog.h"
#include "utils/server.h"

#include <future>

using EnTTidType = entt::registry::entity_type;

using namespace YLineServer;
using namespace drogon_model::yline;

void WorkerCtrl::registerWorkerEnTT(const std::string& workerUUID, const Json::Value& workerInfo, const WebSocketConnectionPtr& wsConnPtr) const
{
    // EnTT
    auto &registry = ServerSingleton::getInstance().Registry;
    //uuid
    const auto& server_instance_uuid = ServerSingleton::getInstance().getServerInstanceUUID();
    // 解析 UUID
    boost::uuids::uuid worker_uuid;
    try {
        worker_uuid = boost::uuids::string_generator()(workerUUID);
    } catch(const std::exception& e) {
        wsConnPtr->shutdown(CloseCode::kUnexpectedCondition, "Failed to parse Worker UUID 解析工作机 UUID 失败");
        spdlog::error("Failed to parse Worker UUID 解析工作机 UUID 失败: {}", e.what());
        return;
    }

    // 注册 worker 实体
    auto workerEntity = registry.create();
    // 添加 worker 元数据组件
    registry.emplace<WorkerMeta>(
        workerEntity,
        worker_uuid,
        server_instance_uuid,
        workerInfo
    );
    // 添加 WebSocket 连接组件
    registry.emplace<WebSocketConnection>(
        workerEntity,
        wsConnPtr
    );
}

EnTTidType findRegisteredWorkerEnTTbyUUIDSync(const boost::uuids::uuid& worker_uuid) noexcept
{
    auto &registry = ServerSingleton::getInstance().Registry;
    auto view = registry.view<WorkerMeta>();
    for (auto entity : view)
    {
        auto& workerMeta = view.get<WorkerMeta>(entity);
        if (workerMeta.worker_uuid == worker_uuid)
        {
            return entity;
        }
    }

    return entt::null;
}

std::shared_future<EnTTidType> findRegisteredWorkerEnTTbyUUIDAsync(const boost::uuids::uuid& worker_uuid)
{
    auto promise = std::make_shared<std::promise<EnTTidType>>();
    auto future = promise->get_future().share(); // 获取共享 future
    // 将任务放入事件循环
    app().getLoop()->queueInLoop([promise, worker_uuid]() {
        auto result = findRegisteredWorkerEnTTbyUUIDSync(worker_uuid);
        promise->set_value(result);
    });

    return future;
}

void WorkerCtrl::registerWorker(const std::string& workerUUID, const Json::Value& workerInfo, const WebSocketConnectionPtr& wsConnPtr) const
{   
    // 发起异步查询 EnTT 注册表， 不阻塞
    auto workerEnTTfuture = findRegisteredWorkerEnTTbyUUIDAsync(boost::uuids::string_generator()(workerUUID));

    auto redis = drogon::app().getFastRedisClient("YLineRedis");
    // database
    auto dbClient = drogon::app().getFastDbClient("YLinedb");
    drogon::orm::Mapper<Workers> mapper(dbClient);


    mapper.findOne(
        drogon::orm::Criteria(Workers::Cols::_worker_uuid, drogon::orm::CompareOperator::EQ, workerUUID),
        [workerUUID, workerEnTTfuture](const Workers &worker) mutable
        {
            spdlog::info("Worker found in database 数据库中找到工作机: {}", workerUUID);

            // 获取  查询结果
            const auto& workerEnTTid = workerEnTTfuture.get();
            if (workerEnTTid != entt::null)
            {
                spdlog::info("Worker found in EnTT registry EnTT 注册表中找到工作机: {}", static_cast<std::underlying_type_t<EnTTidType>>(workerEnTTid));
                return;
            }
            else 
            {
                spdlog::info("Worker not found in EnTT registry EnTT 注册表中未找到工作机: {}", workerUUID);
                // registerNewWorker(workerUUID, workerInfo, wsConnPtr);
            }
        },
        [this, workerUUID, workerInfo, wsConnPtr, workerEnTTfuture](const drogon::orm::DrogonDbException &e) mutable
        {
            spdlog::info("Failed to find Worker in database 数据库中未找到工作机: {}", workerUUID);
            spdlog::info("Exception: {}", e.base().what());

            // 获取 EnTT 查询结果
            const auto& workerEnTTid = workerEnTTfuture.get();
            if (workerEnTTid != entt::null)
            {
                spdlog::info("Worker found in EnTT registry EnTT 注册表中找到工作机: {}", static_cast<std::underlying_type_t<EnTTidType>>(workerEnTTid));
                return;
            }
            else 
            {
                spdlog::info("Worker not found in EnTT registry EnTT 注册表中未找到工作机: {}", workerUUID);
                // registerNewWorker(workerUUID, workerInfo, wsConnPtr);
            }
            // registerNewWorker(workerUUID, workerInfo, wsConnPtr);
        }
    );
    
}

void WorkerCtrl::registerNewWorker(const std::string& workerUUID, const Json::Value& workerInfo, const WebSocketConnectionPtr& wsConnPtr) const
{
    auto redis = drogon::app().getFastRedisClient("YLineRedis");
    // database
    auto dbClient = drogon::app().getFastDbClient("YLinedb");
    drogon::orm::Mapper<Workers> mapper(dbClient);

    //uuid
    const std::string& server_instance_uuid = boost::uuids::to_string(ServerSingleton::getInstance().getServerInstanceUUID());

    // 构造 worker 对象
    Workers worker;
    worker.setWorkerUuid(workerUUID);
    worker.setServerInstanceUuid(server_instance_uuid);
    worker.setWorkerEnttId(33);
     // 将 Json::Value 的 workerInfo 转换为字符串
    Json::StreamWriterBuilder writer;
    const std::string workerInfoStr = Json::writeString(writer, workerInfo);
    worker.setWorkerInfo(workerInfoStr);

    // 插入数据库 这里需要使用同步
    mapper.insert(
        worker,
        [](const Workers worker)
        {
            spdlog::info("New Worker registered into database 新工作机注册到数据库成功: {}", *worker.getWorkerUuid());
        },
        [wsConnPtr](const drogon::orm::DrogonDbException &e)
        {
            wsConnPtr->shutdown(CloseCode::kUnexpectedCondition, "Failed to register new Worker into database 注册新工作机到数据库失败");
            spdlog::error("Failed to register new Worker into database 注册新工作机到数据库失败: {}", e.base().what());
        }
    );
}