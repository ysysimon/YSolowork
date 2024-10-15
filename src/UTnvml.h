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
using NvReturn = nvmlReturn_t;
using NvEnableState = nvmlEnableState_t;
using NvLinkCapability = nvmlNvLinkCapability_t;

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

struct nvUsageInfoGPU {
    double gpuUsage; // GPU 利用率
    double gpuMemoryUsage; // GPU 内存利用率
    double gpuTemperature; // GPU 温度
    double gpuFanSpeed; // GPU 风扇转速
    double gpuClockInfo; // GPU 时钟信息
    double gpuPowerUsage; // GPU 功耗
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
    double PowerLimit = -1.0; // 功耗墙 （W）
    unsigned int TemperatureThreshold = 0.0; // 温度墙
    NVLinkVariant nvLinks;
    // nvUsageInfoGPU usageInfoGPU;
};


// 类: Nvml
class Nvml {
public:
    explicit Nvml();
    ~Nvml();

    // NVlink Variant 回调函数
    // 使用 auto 跨库推导似乎有问题，所以使用明确类型
    // 最好避免在函数参数和函数返回值类型使用 auto
    void handleNVLinkVariant(NVLinkVariant& links, nvDeviceCount deviceIndex);
    
    // 查找设备数量
    nvDeviceCount getDeviceCount();

    // 检查 NVML 返回值, 如果不是 NVML_SUCCESS, 抛出异常
    void nvmlCheckResult(const NvReturn& result);

    // 获取设备句柄
    NvDeviceHandel getDeviceHandle(NvDeviceIndex index);

    // 获取设备名称
    std::string getDeviceName(NvDeviceIndex index);

    // 获取设备序列号
    std::string getDeviceSerial(NvDeviceIndex index);

    // 获取设备驱动版本
    std::string getDeviceDriverVersion(NvDeviceIndex index);

    // 获取设备功耗墙 (milliwatts）
    unsigned int getPowerLimit(NvDeviceIndex index);

    // 获取设备温度墙 (degrees）
    unsigned int getTemperatureThreshold(NvDeviceIndex index);

    // 获取某条 NvLink 状态
    bool getNvLinkState(NvDeviceIndex index, NVLink link);

    // 获取某条 NvLink 版本
    NVLinkVersion getNvLinkVersion(NvDeviceIndex index, NVLink link);

    // 获取某条 NvLink 能力
    std::string getNvLinkCapability(NvDeviceIndex index, NVLink link);


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
};


} // namespace YSolowork::util

#endif // UTnvml_H