#ifndef YLINEWORKER_APP_H
#define YLINEWORKER_APP_H

#include "utils/config.h"
#include "drogon/WebSocketClient.h"
#include <spdlog/spdlog.h>
#include <memory>
#include <trantor/net/EventLoop.h>

#include "UTmachineInfo.h"
#include "UTnvml.h"
#include "UTappdata.h"

#include <boost/uuid/uuid.hpp>

using namespace drogon;

namespace YLineWorker {

// 结构体: worker
struct worker {
    // register secret
    std::string register_secret;

    // worker connection
    drogon::WebSocketClientPtr client;

    // machine info
    YSolowork::util::MachineInfo worker_machineInfo;

};

class WorkerSingleton {
public:
    // 获取单例实例的静态方法
    inline static WorkerSingleton& getInstance() {
        static WorkerSingleton instance;  // 静态局部变量，确保只初始化一次
        return instance;
    }

    // 禁止复制和赋值
    WorkerSingleton(const WorkerSingleton&) = delete;
    WorkerSingleton& operator=(const WorkerSingleton&) = delete;

    // 设置 worker 数据
    inline void setWorkerData(const worker& workerData) {
        workerData_ = workerData;
    }

    // 连接到服务器
    void connectToServer();

    // 初始化 nvml
    inline void initNvml() {
        nvml_.emplace();
        loadNvDevices();
    }

    // 获取 nvml
    inline const std::optional<YSolowork::util::Nvml>& getNvml() const
    {
        return nvml_;
    }

    // 获取 worker usage Json
    Json::Value getUsageJson();

    // 获取 worker usageGPU Json
    Json::Value getUsageGPUJson();

    // 获取 worker register Json
    Json::Value getRegisterJson() const;

    // 获取 worker info Json
    Json::Value getSystomInfoJson() const;

    // 获取 Device Json
    Json::Value getDeviceJson() const;

    // 获取 nvDeviceRegister Json
    Json::Value getNvDeviceRegisterJson() const;
    
    // timer
    trantor::TimerId usageInfotimer;

    // logNvmlInfo
    void logNvmlInfo();

private:
    // 私有构造函数，防止外部实例化
    WorkerSingleton();

    // 存储 worker 信息
    worker workerData_;

    // nvml for Nvidia GPU
    std::optional<YSolowork::util::Nvml> nvml_;

    // nv 设备
    std::optional<std::vector<YSolowork::util::nvDevice>> nvDevices_;

    // 更新 GPU 设备使用信息
    void updateUsageInfoGPU();

    // 加载 nv 设备
    void loadNvDevices();

    // 保存 worker uuid
    static void saveUUIDtoAppData(const boost::uuids::uuid& uuid, const std::filesystem::path& path);

    // 从 app data 读取 uuid
    static boost::uuids::uuid readUUIDfromAppData(const std::filesystem::path& path);

    // worker uuid
    boost::uuids::uuid worker_uuid;
};



void spawnWorker(const Config& config, const std::shared_ptr<spdlog::logger> custom_logger);


using namespace YSolowork::util;

MachineInfo getMachineInfo();

void logWorkerMachineInfo(const MachineInfo& machineInfo);

}
#endif // YLINEWORKER_APP_H