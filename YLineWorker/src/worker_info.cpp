#include "worker.h"
#include <magic_enum.hpp>

namespace YLineWorker {

// ---------------------------- Worker Machine Info ----------------------------

// 通用模板函数，用于执行查询机器信息操作并处理异常
template<typename Func, typename Fallback>
auto executeGetMachineInfo(Func func, Fallback fallback, const std::string& errorMsg, const std::string& warningMsg) -> decltype(func()) 
{
    try {
        return func();
    } catch (const std::exception& e) {
        spdlog::error("{}: {}", errorMsg, e.what());
        spdlog::warn("{}", warningMsg);
        return fallback;
    }
}

MachineInfo getMachineInfo()
{
    MachineInfo machineInfo;

    machineInfo.systomInfo = executeGetMachineInfo(
        []() { return YSolowork::util::getSystomInfo(); },
        SystomInfo{}, // 默认的 fallback 值
        "Failed to get system info 获取系统信息失败", 
        "System information related features may not work properly 系统信息相关功能可能无法正常工作"
    );

    machineInfo.machineName = executeGetMachineInfo(
        []() { return YSolowork::util::getMachineName(); },
        "Unknown", // 默认的 fallback 值
        "Failed to get machine name 获取机器名失败", 
        "Machine name related features may not work properly 机器名相关功能可能无法正常工作"
    );

    machineInfo.devices = executeGetMachineInfo(
        []() { return YSolowork::util::getAllDevices(); },
        std::vector<YSolowork::util::Device>{{
            .type = YSolowork::util::deviceType::Unknown,
            .platformName = "Unknown",
            .name = "Unknown",
            .cores = 0,
            .memoryGB = 0.0
        }},
        "Failed to get devices 获取设备信息失败", 
        "Device related features may not work properly 设备相关功能可能无法正常工作"
    );

    return machineInfo;
}

void logWorkerMachineInfo(const MachineInfo& machineInfo)
{
    spdlog::info("\n---------------------------- Worker Machine Info 工作机器信息 ----------------------------\n\n");
    spdlog::info("\tMachine Name 机器名: {}", machineInfo.machineName);
    spdlog::info("\tSystem 系统: {}", magic_enum::enum_name(machineInfo.systomInfo.os));
    spdlog::info("\tSystem Name 系统名: {}", machineInfo.systomInfo.osName);
    spdlog::info("\tSystem Release 系统发行版: {}", machineInfo.systomInfo.osRelease);
    spdlog::info("\tSystem Version 系统版本号: {}", machineInfo.systomInfo.osVersion);
    spdlog::info("\tSystem Architecture 系统架构: {}", magic_enum::enum_name(machineInfo.systomInfo.osArchitecture));
    spdlog::info("\tDevices 设备:\n");
    for (const auto& device : machineInfo.devices) {
        spdlog::info("\t\tPlatform 平台: {}", device.platformName);
        spdlog::info("\t\tName 名称: {}", device.name);
        spdlog::info("\t\tCores 核心数: {}", device.cores);
        spdlog::info("\t\tMemory 内存: {:.2f} GB", device.memoryGB);
        spdlog::info("\t\tType 类型: {}\n", magic_enum::enum_name(device.type));
    }
    spdlog::info(
        "\n\nYLineWorker needs OpenCL support to get device information, if your device information is not displayed, please check if OpenCL is supported or install runtime Driver.\n"
        "\tGPU: install GPU driver\n"
        "\tx86 CPU: install Intel OpenCL runtime driver (including AMD CPU)\n"
        "\tARM CPU: install ARM OpenCL runtime driver\n"
        "需要 OpenCL 支持以获取设备信息, 如果您的设备信息未显示, 请检查是否支持 OpenCL 或安装运行时驱动程序.\n"
        "\tGPU: 安装 GPU 驱动程序\n"
        "\tx86 CPU: 安装 Intel OpenCL 运行时驱动程序 (包括 AMD CPU)\n"
        "\tARM CPU: 安装 ARM OpenCL 运行时驱动程序\n"
    );
    spdlog::info("\n----------------------------------- Worker Machine Info End 工作机器信息结束 -------------------------------------");
}


// ---------------------------- Worker NVML Info ----------------------------


void WorkerSingleton::logNvmlInfo()
{
    spdlog::info("\n---------------------------- Worker NVML Info 工作机器 NVML 信息 ----------------------------\n\n");
    for(const auto& device : nvDevices_.value()) 
    {
        spdlog::info("\n******************************************************************\n");

        spdlog::info(
        "\nDevice [{}] Information:\n"
            "  Name 设备: {}\n"
            "  Serial 序列号: {}\n"
            "  Driver Version 驱动版本: {}\n"
            "  CUDA Version CUDA 版本: {}\n"
            "  Total VMemory 总显存: {:.2f} GB\n"
            "  Power Limit 功耗墙: {} W\n"
            "  Temperature Threshold 温度墙: {} °C",
            device.index, 
            device.name, 
            device.serial, 
            device.driverVersion, 
            device.cudaVersion, 
            device.totalMemery, 
            device.PowerLimit, 
            device.TemperatureThreshold
        );
        
        auto logVariant = [](const auto& nvLinks) {
            using T = std::decay_t<decltype(nvLinks)>;
            if constexpr (std::is_same_v<T, std::string>) {
                spdlog::info("[✘] NvLink Not Supported 不支持 NvLink");
            } 
            else 
            {
                for (const auto& nvLink : nvLinks) 
                {
                    spdlog::info(
                        "\n[{}] NvLink Supported 支持 NvLink: {}\n"
                        "NvLink Version 版本: {}\n"
                        "NvLink Capability 能力\n{}",
                        nvLink.link, 
                        nvLink.isNvLinkActive,
                        nvLink.NvLinkVersion.value(),
                        nvLink.NvLinkCapability.value()
                    );
                }
            }
        };

        // log NvLink
        std::visit(logVariant, device.nvLinks);
        spdlog::info("\n******************************************************************\n");

    }
    spdlog::info("\n--------------------------------- Worker NVML Info End 工作机器 NVML 信息结束 ----------------------------------------");
}


// 通用模板函数，用于查询 GPU 使用信息并处理异常
template<typename Func, typename FailedCall>
void executeGetUsageInfoGPU(Func func, FailedCall failedcall)
{
    try {
        func();
    } catch (const NVMLException& e) {
        failedcall();
    }
}

void WorkerSingleton::updateUsageInfoGPU()
{
    if (!nvDevices_.has_value()) 
    {
        throw std::runtime_error("Nvidia Devices not loaded, can not update Nvidia GPU usage! 未加载 Nvidia 设备, 无法更新 Nvidia GPU 使用情况!");
    }

    for (auto& device : nvDevices_.value()) 
    {
        // GPU 利用率
        executeGetUsageInfoGPU(
            [this, &device]() { 
                NvUtilization nvUtilization =  nvml_->getDeviceGetUtilizationRates(device.index);
                device.usageInfoGPU.gpuUsage = static_cast<double>(nvUtilization.gpu);
            },
            [this, &device]() {
                device.usageInfoGPU.gpuUsage = -1.0;
            }
        );

        // GPU 内存使用量
        executeGetUsageInfoGPU(
            [this, &device]() { 
                NvMemInfo nvMemInfo = nvml_->getDeviceMemoryInfo(device.index);
                device.usageInfoGPU.gpuMemoryUsed = 
                    static_cast<double>(nvMemInfo.used) 
                    / (1024.0 * 1024.0 * 1024.0); // bytes to GB
            },
            [this, &device]() {
                device.usageInfoGPU.gpuMemoryUsed = -1.0;
            }
        );

        // GPU 温度
        executeGetUsageInfoGPU(
            [this, &device]() { 
                device.usageInfoGPU.gpuTemperature = static_cast<double>(nvml_->getDeviceTemperature(device.index));
            },
            [this, &device]() {
                device.usageInfoGPU.gpuTemperature = -1.0;
            }
        );

        // GPU 时钟信息
        executeGetUsageInfoGPU(
            [this, &device]() { 
                nvClockInfo nvClockInfo = nvml_->getDeviceAllClockInfo(device.index);
                device.usageInfoGPU.gpuClockInfo = {
                    .graphicsClock = static_cast<double>(nvClockInfo.graphicsClock),
                    .smClock = static_cast<double>(nvClockInfo.smClock),
                    .memClock = static_cast<double>(nvClockInfo.memClock),
                    .videoClock = static_cast<double>(nvClockInfo.videoClock)
                };
            },
            [this, &device]() {
                device.usageInfoGPU.gpuClockInfo = {
                    .graphicsClock = -1.0,
                    .smClock = -1.0,
                    .memClock = -1.0,
                    .videoClock = -1.0
                };
            }
        );

        // GPU 功耗
        executeGetUsageInfoGPU(
            [this, &device]() { 
                device.usageInfoGPU.gpuPowerUsage = 
                    static_cast<double>(nvml_->getDevicePowerUsage(device.index)) / 1000.0 ; // milliwatts to watts
            },
            [this, &device]() {
                device.usageInfoGPU.gpuPowerUsage = -1.0;
            }
        );

    }
}


template<typename Func, typename Fallback>
auto executeGetNVDevice(Func func, Fallback fallback, const std::string& errorMsg, const std::string& warningMsg) -> decltype(func()) {
    try {
        return func();
    } catch (const NVMLException& e) { // 如果只处理 NVMLException，可以特化处理
        spdlog::warn("{}: {}", errorMsg, e.what());
        spdlog::warn("{}", warningMsg);
        return fallback;
    }
}

void WorkerSingleton::loadNvDevices()
{

    if (!nvml_.has_value()) 
    {
        std::runtime_error("NVML not initialized NVML, can not load Nvidia Device! 未初始化 NVML, 无法加载 Nvidia 设备!");
    }

    nvDevices_ = std::vector<YSolowork::util::nvDevice>();

    spdlog::info("\n\n---------------------- Loading Nvidia Device 加载 Nvidia 设备 ----------------------\n\n");

    // 获取设备数量
    nvDeviceCount deviceCount = executeGetNVDevice(
        [this]() { return nvml_->getDeviceCount(); },
        0,
        "Failed to get device count 获取设备数量失败",
        "This machine may not have Nvidia GPU 本机可能没有 Nvidia GPU"
    );

    // 遍历设备
    for (nvDeviceCount currDevice = 0; currDevice < deviceCount; currDevice++) {
        nvDevice device;
        device.index = currDevice;

        spdlog::info("*********** Loading device - [{}] 加载设备 [{}] 开始 ***********", currDevice, currDevice);

        device.name = executeGetNVDevice(
            [this, currDevice]() { return nvml_->getDeviceName(currDevice); },
            std::string("Unknown"),
            "Failed to get device name 获取设备名称失败",
            "This device may not support name feature 本设备可能不支持名称功能"
        );

        device.serial = executeGetNVDevice(
            [this, currDevice]() { return nvml_->getDeviceSerial(currDevice); },
            std::string("Unsupport"),
            "Failed to get device serial 获取设备序列号失败",
            "This device may not support serial feature 本设备可能不支持序列号功能"
        );

        device.cudaVersion = executeGetNVDevice(
            [this, currDevice]() { return nvml_->getSystemCudaDriverVersion(); },
            -1.0,
            "Failed to get system CUDA driver version 获取系统 CUDA 驱动版本失败",
            "This device may not support CUDA feature 本设备可能不支持 CUDA 功能"
        );

        device.totalMemery = executeGetNVDevice(
            [this, currDevice]() { 
                return static_cast<double>(
                    nvml_->getDeviceMemoryInfo(currDevice).total
                    ) / (1024 * 1024 * 1024); // bytes to GB
            }, 
            -1.0,
            "Failed to get device Vmemory info 获取设备显存信息失败",
            "This device may not support Vmemory info feature 本设备可能不支持显存信息功能"
        );

        device.driverVersion = executeGetNVDevice(
            [this, currDevice]() { return nvml_->getDeviceDriverVersion(currDevice); },
            std::string("Unsupport"),
            "Failed to get device driver version 获取设备驱动版本失败",
            "This device may not support driver version feature 本设备可能不支持驱动版本功能"
        );

        device.PowerLimit = executeGetNVDevice(
            [this, currDevice]() { return nvml_->getPowerLimit(currDevice) / 1000; }, // milliwatts to watts
            0.0,
            "Failed to get power limit 获取功耗墙失败",
            "This device may not support power limit feature 本设备可能不支持功耗墙功能"
        );

        device.TemperatureThreshold = executeGetNVDevice(
            [this, currDevice]() { return nvml_->getTemperatureThreshold(currDevice); },
            0,
            "Failed to get temperature threshold 获取温度墙失败",
            "This device may not support temperature threshold feature 本设备可能不支持温度墙功能"
        );

        // 如果支持 NvLink, 设置 variant 为 nvLink 数组
        bool get_nvLinkState_result = executeGetNVDevice(
            [this, currDevice]() { nvml_->getNvLinkState(currDevice, 0); return true; },
            false,
            "Failed to get NvLink state 获取 NvLink 状态失败",
            "This device may not support NvLink feature 本设备可能不支持 NvLink 功能"
        );

        if (get_nvLinkState_result) {
            device.nvLinks = std::vector<nvLink>();
        } 

        // set NvLink variant
        nvml_->handleNVLinkVariant(device.nvLinks, currDevice);

        nvDevices_->push_back(device);

        spdlog::info("*********** Loading device End - [{}] 加载 [{}] 结束 ***********", currDevice, currDevice);
    }

    spdlog::info("\n\n------------------------------- Loading Nvidia Device End 加载 Nvidia 设备结束 ------------------------------------\n\n");
}


} // namespace YLineWorker