#ifndef UTnvml_H
#define UTnvml_H
#include "UTdynlib.h"

#include <array>
#include <variant>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "nvml.h"

namespace YSolowork::util {

using NvDeviceHandel = nvmlDevice_t;
using NvDeviceIndex = unsigned int;
using nvDeviceCount = unsigned int;
using NVLink = unsigned int;
using NVLinkVersion = unsigned int;
using NvLinkCapabilityResult = unsigned int;
using NvDeviceTemperature = unsigned int;
using NvDeviceFanSpeed = unsigned int;
using NvFanIndex = unsigned int;
using NvFanNum = unsigned int;
using NvClock = unsigned int;
using NvPower = unsigned int;
using cudaDriverVersion = int;
using NvReturn = nvmlReturn_t;
using NvEnableState = nvmlEnableState_t;
using NvLinkCapability = nvmlNvLinkCapability_t;
using NvUtilization = nvmlUtilization_t;
using NvClockType = nvmlClockType_t;
using NvMemInfo = nvmlMemory_t;

// 异常类: NVML异常
class NVMLException : public std::exception {
public:
    inline explicit NVMLException(const std::string& message) : msg_(message) {}

    // 覆盖 what() 方法，返回错误信息
    inline const char* what() const noexcept override
    {
        return msg_.c_str();
    }

private:
    std::string msg_;
};

// 这个是定义在 nvml.h 中的结构体
// typedef struct nvmlUtilization_st
// {
//     unsigned int gpu;                //!< Percent of time over the past sample period during which one or more kernels was executing on the GPU
//     unsigned int memory;             //!< Percent of time over the past sample period during which global (device) memory was being read or written
// } nvmlUtilization_t;

// 这个也是定义在 nvml.h 中的结构体
// typedef struct nvmlMemory_v2_st
// {
//     unsigned int version;            //!< Structure format version (must be 2)
//     unsigned long long total;        //!< Total physical device memory (in bytes)
//     unsigned long long reserved;     //!< Device memory (in bytes) reserved for system use (driver or firmware)
//     unsigned long long free;         //!< Unallocated device memory (in bytes)
//     unsigned long long used;         //!< Allocated device memory (in bytes). Note that the driver/GPU always sets aside a small amount of memory for bookkeeping
// } nvmlMemory_v2_t;

struct nvFan {
    NvFanIndex index;
    double fanSpeed = 0.0; // 风扇转速
};

struct nvClockInfo { // (MHz)
    NvClock graphicsClock; // 核心时钟
    NvClock smClock; // 显存时钟
    NvClock memClock; // 内存时钟
    NvClock videoClock; // 视频时钟
};

struct nvClockInfoD {
    double graphicsClock = 0.0; // 核心时钟
    double smClock = 0.0; // 显存时钟
    double memClock = 0.0; // 内存时钟
    double videoClock = 0.0; // 视频时钟
};

struct nvUsageInfoGPU {
    double gpuUsage = 0.0; // GPU 利用率
    double gpuMemoryUsed = 0.0; // GPU 内存可用量 (GB)
    double gpuTemperature = 0.0; // GPU 温度
    nvClockInfoD gpuClockInfo; // GPU 时钟信息
    double gpuPowerUsage = 0.0; // GPU 功耗
};

struct nvLink {
    NVLink link;
    bool isNvLinkSupported = false; // 是否支持 NvLink
    std::optional<NVLinkVersion> NvLinkVersion; // NvLink 版本
    std::optional<std::string> NvLinkCapability; // NvLink 能力
};

using NVLinkVariant = std::variant<std::string, std::array<nvLink, NVML_NVLINK_MAX_LINKS>>;

struct nvDevice {
    unsigned int index;
    std::string name;
    std::string serial;
    std::string driverVersion;
    float cudaVersion;
    double totalMemery; // 总内存 (GB)
    NvPower PowerLimit; // 功耗墙 （W）
    NvDeviceTemperature TemperatureThreshold; // 温度墙
    NVLinkVariant nvLinks;
    std::optional<std::vector<nvFan>> fans;
    nvUsageInfoGPU usageInfoGPU;
};


// 类: Nvml
class Nvml {
public:
    explicit Nvml();
    ~Nvml();

    // NVlink Variant 回调函数
    // 使用 auto 跨库推导似乎有问题，所以使用明确类型
    // 最好避免在函数参数和函数返回值类型使用 auto
    void handleNVLinkVariant(NVLinkVariant& links, nvDeviceCount deviceIndex) const;
    
    // 查找设备数量
    nvDeviceCount getDeviceCount() const;

    // 检查 NVML 返回值, 如果不是 NVML_SUCCESS, 抛出异常
    void nvmlCheckResult(const NvReturn& result) const;

    // 获取设备句柄
    NvDeviceHandel getDeviceHandle(NvDeviceIndex index) const;

    // 获取设备名称
    std::string getDeviceName(NvDeviceIndex index) const;

    // 获取设备序列号
    std::string getDeviceSerial(NvDeviceIndex index) const;

    // 获取设备驱动版本
    std::string getDeviceDriverVersion(NvDeviceIndex index) const;

    // 获取设备功耗墙 (milliwatts）
    NvPower getPowerLimit(NvDeviceIndex index) const;

    // 获取设备温度墙 (degrees）
    NvDeviceTemperature getTemperatureThreshold(NvDeviceIndex index) const;

    // 获取某条 NvLink 状态
    bool getNvLinkState(NvDeviceIndex index, NVLink link) const;

    // 获取某条 NvLink 版本
    NVLinkVersion getNvLinkVersion(NvDeviceIndex index, NVLink link) const;

    // 获取某条 NvLink 能力
    std::string getNvLinkCapability(NvDeviceIndex index, NVLink link) const;

    // 获取设备利用率
    NvUtilization getDeviceGetUtilizationRates(NvDeviceIndex index) const;

    // 获取设备当前温度
    NvDeviceTemperature getDeviceTemperature(NvDeviceIndex index)  const;

    // 获取风扇转速
    NvDeviceFanSpeed getDeviceFanSpeed(NvDeviceIndex index, NvFanIndex fanIndex) const;

    // 获取风扇数量
    NvFanNum getDeviceFanNum(NvDeviceIndex index) const;

    // 获取设备当前时钟信息 (MHz)
    NvClock getDeviceClockInfo(NvDeviceIndex index, NvClockType type) const;

    // 获取所有可获取的时钟信息
    nvClockInfo getDeviceAllClockInfo(NvDeviceIndex index) const;

    // 获取设备当前功耗 (mW)
    NvPower getDevicePowerUsage(NvDeviceIndex index) const;

    // 获取系统 CUDA 驱动版本
    float getSystemCudaDriverVersion() const;

    // 获取设备内存信息
    NvMemInfo getDeviceMemoryInfo(NvDeviceIndex index) const;

private:
    YSolowork::util::DynamicLibrary loader;
    std::vector<nvDevice> devices;

    // NVML 函数的 std::function 封装
    std::function<nvmlReturn_t()> 
    nvmlInit; // 初始化 NVML
    std::function<nvmlReturn_t()> 
    nvmlShutdown; // 关闭 NVML
    std::function<nvmlReturn_t(unsigned int, nvmlDevice_t*)> 
    nvmlDeviceGetHandleByIndex; // 获取设备句柄
    std::function<nvmlReturn_t(unsigned int *)>
    nvmlDeviceGetCount; // 获取设备数量
    std::function<nvmlReturn_t(nvmlDevice_t, char*, unsigned int)>
    nvmlDeviceGetName; // 获取设备名称
    std::function<nvmlReturn_t(nvmlDevice_t, nvmlUtilization_t*)> 
    nvmlDeviceGetUtilizationRates; // 获取设备利用率
    std::function<const char*(nvmlReturn_t)> 
    nvmlErrorString; // 获取错误信息
    std::function<nvmlReturn_t(nvmlDevice_t, char*, unsigned int)> 
    nvmlDeviceGetSerial; // 获取设备序列号
    std::function<nvmlReturn_t(char*, unsigned int)> 
    nvmlSystemGetDriverVersion; // 获取设备驱动版本
    std::function<nvmlReturn_t(nvmlDevice_t, unsigned int*)> 
    nvmlDeviceGetEnforcedPowerLimit; // 获取设备功耗墙
    std::function<nvmlReturn_t(nvmlDevice_t, nvmlTemperatureThresholds_t, unsigned int*)> 
    nvmlDeviceGetTemperatureThreshold; // 获取设备温度墙
    std::function<nvmlReturn_t(nvmlDevice_t, unsigned int, nvmlEnableState_t *)>
    nvmlDeviceGetNvLinkState; // 获取设备 NvLink 状态
    std::function<nvmlReturn_t(nvmlDevice_t, unsigned int, unsigned int *)>
    nvmlDeviceGetNvLinkVersion; // 获取设备 NvLink 版本
    std::function<nvmlReturn_t(nvmlDevice_t, unsigned int, nvmlNvLinkCapability_t, unsigned int *)>
    nvmlDeviceGetNvLinkCapability; // 获取设备 NvLink 能力
    std::function<nvmlReturn_t(nvmlDevice_t, nvmlTemperatureSensors_t, unsigned int *)>
    nvmlDeviceGetTemperature; // 获取设备当前温度
    std::function<nvmlReturn_t(nvmlDevice_t, unsigned int, unsigned int *)>
    nvmlDeviceGetFanSpeed_v2; // 获取设备当前风扇的应该转速 (无法从此值发现风扇是否物理阻挡)
    std::function<nvmlReturn_t(nvmlDevice_t, unsigned int *)>
    nvmlDeviceGetNumFans; // 获取设备风扇数量
    std::function<nvmlReturn_t(nvmlDevice_t, nvmlClockType_t, unsigned int *)>
    nvmlDeviceGetClockInfo; // 获取设备当前时钟信息 (MHz)
    std::function<nvmlReturn_t(nvmlDevice_t, unsigned int *)>
    nvmlDeviceGetPowerUsage; // 获取设备当前功耗
    std::function<nvmlReturn_t(int *)>
    nvmlSystemGetCudaDriverVersion_v2; // 获取系统 CUDA 驱动版本
    std::function<nvmlReturn_t(nvmlDevice_t, nvmlMemory_t *)>
    nvmlDeviceGetMemoryInfo; // 获取设备内存信息

};


} // namespace YSolowork::util

#endif // UTnvml_H