#include "YLineServer_WorkerCtrl.h"

#include "entt/entity/fwd.hpp"
#include "models/Workers.h"
#include "spdlog/spdlog.h"
#include "utils/server.h"
#include "utils/api.h"

#include <boost/uuid/string_generator.hpp>
#include <future>
#include <json/value.h>

using EnTTidType = entt::registry::entity_type;

using namespace YLineServer;
using namespace drogon_model::yline;

// void WorkerCtrl::registerWorkerEnTT(const std::string& workerUUID, const Json::Value& workerInfo, const WebSocketConnectionPtr& wsConnPtr) const
// {
//     // EnTT
//     auto &registry = ServerSingleton::getInstance().Registry;
//     //uuid
//     const auto& server_instance_uuid = ServerSingleton::getInstance().getServerInstanceUUID();
//     // 解析 UUID
//     boost::uuids::uuid worker_uuid;
//     try {
//         worker_uuid = boost::uuids::string_generator()(workerUUID);
//     } catch(const std::exception& e) {
//         wsConnPtr->shutdown(CloseCode::kUnexpectedCondition, "Failed to parse Worker UUID 解析工作机 UUID 失败");
//         spdlog::error("Failed to parse Worker UUID 解析工作机 UUID 失败: {}", e.what());
//         return;
//     }

//     // 注册 worker 实体
//     auto workerEntity = registry.create();
//     // 添加 worker 元数据组件
//     registry.emplace<WorkerMeta>(
//         workerEntity,
//         worker_uuid,
//         server_instance_uuid,
//         workerInfo
//     );
//     // 添加 WebSocket 连接组件
//     registry.emplace<WSConnection>(
//         workerEntity,
//         wsConnPtr
//     );
// }


void registerNewWorkerDatabase(
    const std::string& workerUUID, 
    const Json::Value& workerInfo, 
    EnTTidType workerEnTTid,
    const WebSocketConnectionPtr& wsConnPtr
)
{
    // database
    auto dbClient = drogon::app().getFastDbClient("YLinedb");
    drogon::orm::Mapper<Workers> mapper(dbClient);

    //uuid
    const std::string& server_instance_uuid = boost::uuids::to_string(ServerSingleton::getInstance().getServerInstanceUUID());

    // 构造 worker 对象
    Workers worker;
    worker.setWorkerUuid(workerUUID);
    worker.setServerInstanceUuid(server_instance_uuid);
    worker.setWorkerEnttId(static_cast<std::underlying_type_t<EnTTidType>>(workerEnTTid));
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
        auto it = ServerSingleton::getInstance().WorkerUUIDtoEnTTid.find(worker_uuid);
        // 首先在 hash 表中查找
        if (it != ServerSingleton::getInstance().WorkerUUIDtoEnTTid.end())
        {
            promise->set_value(it->second);
            return;
        }

        // 旧代码 也许冗余 但是保留
        auto result = findRegisteredWorkerEnTTbyUUIDSync(worker_uuid);
        promise->set_value(result);
    });

    return future;
}

EnTTidType registerNewWorkerEnTT(
    const boost::uuids::uuid& worker_uuid, 
    const boost::uuids::uuid& server_instance_uuid, 
    const Json::Value& workerInfo, 
    const WebSocketConnectionPtr& wsConnPtr
)
{
    auto &registry = ServerSingleton::getInstance().Registry;
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
    registry.emplace<WSConnection>(
        workerEntity,
        wsConnPtr
    );

    return workerEntity;
}

Workers getUpdateWorker(
    const Workers& oldWorker, 
    const Json::Value& workerInfo, 
    const boost::uuids::uuid& server_instance_uuid,
    EnTTidType workerEnTTid
)
{
    Workers updateWorker = oldWorker;
    updateWorker.setWorkerInfo(Json::writeString(Json::StreamWriterBuilder(), workerInfo));
    updateWorker.setServerInstanceUuid(boost::uuids::to_string(server_instance_uuid));
    updateWorker.setWorkerEnttId(static_cast<std::underlying_type_t<EnTTidType>>(workerEnTTid));
    return updateWorker;
}

void updateWorkerDatabase(
    std::shared_ptr<drogon::orm::Mapper<Workers>> mapper, 
    const Workers updateWorker, 
    const std::string& workerUUID, 
    const WebSocketConnectionPtr& wsConnPtr
)
{
    mapper->update(
        updateWorker,
        [workerUUID](const size_t count)
        {
            spdlog::info("Worker updated in database 数据库中工作机更新成功: {}", workerUUID);
        },
        [wsConnPtr](const drogon::orm::DrogonDbException &e)
        {
            wsConnPtr->shutdown(CloseCode::kUnexpectedCondition, "Failed to update Worker into database 更新工作机到数据库失败");
            spdlog::error("Failed to update Worker into database 更新工作机到数据库失败: {}", e.base().what());
        }
    );
}

void WorkerCtrl::registerWorker(const std::string& workerUUID, const Json::Value& workerInfo, const WebSocketConnectionPtr& wsConnPtr) const
{   
    // 发起异步查询 EnTT 注册表， 不阻塞
    auto workerEnTTfuture = findRegisteredWorkerEnTTbyUUIDAsync(boost::uuids::string_generator()(workerUUID));

    // database
    auto dbClient = drogon::app().getFastDbClient("YLinedb");
    auto mapper = std::make_shared<drogon::orm::Mapper<Workers>>(dbClient);

    // 在数据库中查询工作机，如果则更新，否则注册新工作机
    mapper->findOne(
        drogon::orm::Criteria(Workers::Cols::_worker_uuid, drogon::orm::CompareOperator::EQ, workerUUID),
        [mapper, workerUUID, workerInfo, workerEnTTfuture, wsConnPtr](const Workers& databaseWorker) mutable
        {
            spdlog::info("Worker found in database 数据库中找到工作机: {}", workerUUID);

            // 获取  查询结果
            const auto& workerEnTTid = workerEnTTfuture.get();
            if (workerEnTTid != entt::null)
            {
                // 数据库和 EnTT 注册表中都找到了工作机, 则说明工作机之前就注册在本实例，所以更新数据库中的 worker 数据为本实例
                spdlog::info("Worker found in EnTT registry EnTT 注册表中找到工作机: {}", static_cast<std::underlying_type_t<EnTTidType>>(workerEnTTid));
                Workers updateWorker = getUpdateWorker(
                    databaseWorker, 
                    workerInfo, 
                    ServerSingleton::getInstance().getServerInstanceUUID(),
                    workerEnTTid
                );
                updateWorkerDatabase(mapper, updateWorker, workerUUID, wsConnPtr);
            }
            else 
            {
                // 数据库中找到了工作机，但是 EnTT 注册表中未找到，说明工作机之前未注册在本实例，所以注册新工作机到 EnTT 注册表，并更新数据库中的 worker 数据为本实例
                spdlog::info("Worker not found in EnTT registry EnTT 注册表中未找到工作机: {}", workerUUID);

                // 注册新工作机到 EnTT 注册表
                boost::uuids::string_generator gen;
                boost::uuids::uuid databaseWorkerUUID = gen(*databaseWorker.getWorkerUuid());
                boost::uuids::uuid databaseWorker_server_instance_uuid = gen(*databaseWorker.getServerInstanceUuid());

                const auto newWorkerEntity = registerNewWorkerEnTT(
                    databaseWorkerUUID, 
                    databaseWorker_server_instance_uuid, 
                    workerInfo, 
                    wsConnPtr
                );

                // 同时添加到 hash 表
                ServerSingleton::getInstance().WorkerUUIDtoEnTTid[databaseWorkerUUID] = newWorkerEntity;

                spdlog::info(
                    "New Worker registered into EnTT registry 新工作机注册到 EnTT 注册表成功: {}", 
                    static_cast<std::underlying_type_t<EnTTidType>>(newWorkerEntity)
                );

                // 更新数据库中的 worker 数据
                Workers updateWorker = getUpdateWorker(
                    databaseWorker, 
                    workerInfo, 
                    ServerSingleton::getInstance().getServerInstanceUUID(),
                    newWorkerEntity
                );

                updateWorkerDatabase(mapper, updateWorker, workerUUID, wsConnPtr);
                                
            }
        },
        [this, workerUUID, workerInfo, wsConnPtr, workerEnTTfuture](const drogon::orm::DrogonDbException &e) mutable
        {
            spdlog::info("Failed to find Worker in database 数据库中未找到工作机: {}", workerUUID);
            spdlog::debug("Database Exception: {}", e.base().what());

            // 获取 EnTT 查询结果
            auto workerEnTTid = workerEnTTfuture.get();
            if (workerEnTTid != entt::null)
            {
                // 数据库中未找到工作机，但是 EnTT 注册表中找到了，说明工作机之前就注册在本实例，所以注册新工作机到数据库
                spdlog::info("Worker found in EnTT registry EnTT 注册表中找到工作机: {}", static_cast<std::underlying_type_t<EnTTidType>>(workerEnTTid));
            }
            else 
            {
                // 数据库和 EnTT 注册表中都未找到工作机, 则说明工作机之前未注册在本实例，所以注册新工作机到 EnTT 注册表，并注册到数据库
                spdlog::info("Worker not found in EnTT registry EnTT 注册表中未找到工作机: {}", workerUUID);
                boost::uuids::string_generator gen;
                boost::uuids::uuid newWorkerUUID = gen(workerUUID);
                workerEnTTid = registerNewWorkerEnTT(
                    newWorkerUUID,
                    ServerSingleton::getInstance().getServerInstanceUUID(), 
                    workerInfo, 
                    wsConnPtr
                );

                // 同时添加到 hash 表
                ServerSingleton::getInstance().WorkerUUIDtoEnTTid[newWorkerUUID] = workerEnTTid;

                spdlog::info(
                    "New Worker registered into EnTT registry 新工作机注册到 EnTT 注册表成功: {}", 
                    static_cast<std::underlying_type_t<EnTTidType>>(workerEnTTid)
                );
            }
            // 注册新工作机到数据库 该函使用本实例 UUID
            registerNewWorkerDatabase(workerUUID, workerInfo, workerEnTTid, wsConnPtr);
        }
    );
    
}