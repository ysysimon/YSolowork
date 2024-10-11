#ifndef YLINEWORKER_APP_H
#define YLINEWORKER_APP_H

#include "utils/config.h"
#include "drogon/WebSocketClient.h"
#include <spdlog/spdlog.h>
#include <memory>
#include <trantor/net/EventLoop.h>

#include "UTmachineInfo.h"

namespace YLineWorker {

// 结构体: worker
struct worker {
    // worker info
    std::string worker_id;
    std::string worker_name;

    // worker connection
    drogon::WebSocketClientPtr client;

    // machine info
    YSolowork::untility::MachineInfo worker_machineInfo;

};

class WorkerSingleton {
public:
    // 获取单例实例的静态方法
    static WorkerSingleton& getInstance();

    // 禁止复制和赋值
    WorkerSingleton(const WorkerSingleton&) = delete;
    WorkerSingleton& operator=(const WorkerSingleton&) = delete;

    // 获取 worker 数据
    const worker& getWorkerData() const;

    // 设置 worker 数据
    void setWorkerData(const worker& workerData);

    // 连接到服务器
    void connectToServer();
    
    // timer
    trantor::TimerId usageInfoCPUtimer;
private:
    // 私有构造函数，防止外部实例化
    WorkerSingleton() = default;

    // 存储 worker 信息
    worker workerData_;
};



void spawnWorker(const Config& config, const std::shared_ptr<spdlog::logger> custom_logger);


using namespace YSolowork::untility;

MachineInfo getMachineInfo();

void logWorkerMachineInfo(const MachineInfo& machineInfo);

}
#endif // YLINEWORKER_APP_H