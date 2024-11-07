#include "worker.h"
#include "json/value.h"
#include <magic_enum.hpp>
#include <variant>
#include "UTusage.h"

#include <boost/uuid/uuid_io.hpp> // for boost::uuids::to_string

namespace YLineWorker {


Json::Value WorkerSingleton::getUsageJson()
{
    UsageInfoCPU usageInfoCPU = YSolowork::util::getUsageInfoCPU();
    Json::Value json;
    json["command"] = "usage";
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

// Json::Value WorkerSingleton::getDeviceJson() const
// {
//     Json::Value json;
//     for (const auto& device : workerData_.worker_machineInfo.devices) 
//     {
//         Json::Value deviceJson;
//         deviceJson["type"] = std::string(
//             magic_enum::enum_name(device.type)
//         );
//         deviceJson["platformName"] = device.platformName;
//         deviceJson["name"] = device.name;
//         deviceJson["cores"] = device.cores;
//         deviceJson["memoryGB"] = device.memoryGB;
//         json.append(deviceJson);
//     }

//     return json;
// }

// Json::Value WorkerSingleton::getSystomInfoJson() const
// {
//     Json::Value json;
//     json["OS"] = std::string(
//          magic_enum::enum_name(workerData_.worker_machineInfo.systomInfo.os)
//     );
//     json["osName"] = workerData_.worker_machineInfo.systomInfo.osName;
//     json["osRelease"] = workerData_.worker_machineInfo.systomInfo.osRelease;
//     json["osVersion"] = workerData_.worker_machineInfo.systomInfo.osVersion;
//     json["osArchitecture"] = std::string(
//          magic_enum::enum_name(workerData_.worker_machineInfo.systomInfo.osArchitecture)
//     );


//     return json;
// }

Json::Value WorkerSingleton::getMachineInfoJson() const
{
    Json::Value json;
    json["machineName"] = workerData_.worker_machineInfo.machineName;
    json["systomInfo"]["OS"] = std::string(
         magic_enum::enum_name(workerData_.worker_machineInfo.systomInfo.os)
    );
    json["systomInfo"]["osName"] = workerData_.worker_machineInfo.systomInfo.osName;
    json["systomInfo"]["osRelease"] = workerData_.worker_machineInfo.systomInfo.osRelease;
    json["systomInfo"]["osVersion"] = workerData_.worker_machineInfo.systomInfo.osVersion;
    json["systomInfo"]["osArchitecture"] = std::string(
         magic_enum::enum_name(workerData_.worker_machineInfo.systomInfo.osArchitecture)
    );
    for (const auto& device : workerData_.worker_machineInfo.devices) 
    {
        Json::Value deviceJson;
        deviceJson["type"] = std::string(
            magic_enum::enum_name(device.type)
        );
        deviceJson["platformName"] = device.platformName;
        deviceJson["name"] = device.name;
        deviceJson["cores"] = device.cores;
        deviceJson["memoryGB"] = device.memoryGB;
        json["devices"].append(deviceJson);
    }

    return json;
}

Json::Value WorkerSingleton::getNvDeviceRegisterJson() const
{
    auto nvLinkJsonVariant = [](const auto& nvLink) -> Json::Value 
    {
        using T = std::decay_t<decltype(nvLink)>;
        if constexpr (std::is_same_v<T, std::string>) 
        {
            return Json::Value(nvLink);
        } 
        else if constexpr (std::is_same_v<T, std::vector<YSolowork::util::nvLink>>) 
        {
            Json::Value json;
            for (const auto& link : nvLink) 
            {
                Json::Value linkJson;
                linkJson["link"] = link.link;
                linkJson["isNvLinkActive"] = link.isNvLinkActive;
                if (link.NvLinkVersion.has_value()) 
                {
                    linkJson["NvLinkVersion"] = link.NvLinkVersion.value();
                }
                if (link.NvLinkCapability.has_value()) 
                {
                    linkJson["NvLinkCapability"] = link.NvLinkCapability.value();
                }
                json.append(linkJson);
            }
            return json;
        }
    };

    Json::Value json;
    for (const auto& device : nvDevices_.value()) 
    {
        Json::Value deviceJson;
        deviceJson["index"] = device.index;
        deviceJson["name"] = device.name;
        deviceJson["serial"] = device.serial;
        deviceJson["driverVersion"] = device.driverVersion;
        deviceJson["cudaVersion"] = device.cudaVersion;
        deviceJson["totalMemery"] = device.totalMemery;
        deviceJson["PowerLimit"] = device.PowerLimit;
        deviceJson["TemperatureThreshold"] = device.TemperatureThreshold;
        deviceJson["nvLinks"] = 
            std::visit(nvLinkJsonVariant, device.nvLinks);
        json.append(deviceJson);
    }

    return json;
}

Json::Value WorkerSingleton::getRegisterJson() const
{
    Json::Value json;
    json["worker_uuid"] = boost::uuids::to_string(worker_uuid);
    json["register_secret"] = workerData_.register_secret;
    json["worker_info"]["machineInfo"] = getMachineInfoJson();

    if (nvml_.has_value() && nvDevices_.has_value() && !nvDevices_.value().empty()) 
    {
        json["worker_info"]["NVIDIA"] = getNvDeviceRegisterJson();
    }

    return json;
}


} // namespace YLineWorker