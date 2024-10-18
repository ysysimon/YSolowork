#include "worker.h"
#include <magic_enum.hpp>
#include "UTusage.h"

namespace YLineWorker {


Json::Value WorkerSingleton::getUsageJson()
{
    UsageInfoCPU usageInfoCPU = YSolowork::util::getUsageInfoCPU();
    Json::Value json;
    json["cpuUsage"] = usageInfoCPU.cpuUsage;
    json["cpuMemoryUsage"] = usageInfoCPU.memoryUsage;

    if (nvml_.has_value()) 
    {   
        json["gpuUsage"] = getUsageGPUJson();
    }

    return json;
}

Json::Value WorkerSingleton::getUsageGPUJson()
{
    
    if (!nvml_.has_value()) 
    {
        throw std::runtime_error("NVML not initialized NVML, can not get Nvidia GPU usage! 未初始化 NVML, 无法获取 Nvidia GPU 情况!");
    }

    updateUsageInfoGPU();
    Json::Value json;
    for (const auto& device : nvDevices_.value()) 
    {
        Json::Value deviceJson;
        deviceJson["index"] = device.index;
        // deviceJson["name"] = device.name;
        deviceJson["gpuUsage"] = device.usageInfoGPU.gpuUsage;
        deviceJson["gpuMemoryUsed"] = device.usageInfoGPU.gpuMemoryUsed;
        deviceJson["gpuTemperature"] = device.usageInfoGPU.gpuTemperature;
        deviceJson["gpuClockInfo"]["graphicsClock"] = device.usageInfoGPU.gpuClockInfo.graphicsClock;
        deviceJson["gpuClockInfo"]["smClock"] = device.usageInfoGPU.gpuClockInfo.smClock;
        deviceJson["gpuClockInfo"]["memClock"] = device.usageInfoGPU.gpuClockInfo.memClock;
        deviceJson["gpuClockInfo"]["videoClock"] = device.usageInfoGPU.gpuClockInfo.videoClock;
        deviceJson["gpuPowerUsage"] = device.usageInfoGPU.gpuPowerUsage;
        json["NVIDIA"].append(deviceJson);
    }

    return json;
}

Json::Value WorkerSingleton::getSystomInfoJson() const
{
    Json::Value json;
    json["OS"] = std::string(
        magic_enum::enum_name(workerData_.worker_machineInfo.systomInfo.os)
    );
        json["osName"] = workerData_.worker_machineInfo.systomInfo.osName;
        json["osRelease"] = workerData_.worker_machineInfo.systomInfo.osRelease;
        json["osVersion"] = workerData_.worker_machineInfo.systomInfo.osVersion;
        json["osArchitecture"] = std::string(
        magic_enum::enum_name(workerData_.worker_machineInfo.systomInfo.osArchitecture)
    );

    return json;
}

Json::Value WorkerSingleton::getRegisterJson() const
{
    Json::Value json;
    json["worker_id"] = workerData_.worker_id;
    json["worker_name"] = workerData_.worker_name;
    json["worker_machineInfo"] = getSystomInfoJson();
    return json;
}


} // namespace YLineWorker