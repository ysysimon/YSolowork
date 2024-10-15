#include "UTnvml.h"
#include "nvml.h"
#include <string>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__linux__)
#include <dlfcn.h>
#else
#error "Unsupported platform"
#endif

namespace YSolowork::util {

Nvml::Nvml() : loader(
#ifdef _WIN32
    YSolowork::util::DynamicLibrary("nvml.dll")
#else
    YSolowork::util::DynamicLibrary("libnvidia-ml.so")
#endif
)
{

    loader.load();

    // 获取函数指针并存储在 std::function 中
    nvmlInit = loader.getFunction<nvmlReturn_t()>("nvmlInit_v2");

    nvmlShutdown = loader.getFunction<nvmlReturn_t()>("nvmlShutdown");

    nvmlDeviceGetHandleByIndex = loader.getFunction<nvmlReturn_t(unsigned int, nvmlDevice_t*)>
        ("nvmlDeviceGetHandleByIndex_v2");

    nvmlDeviceGetCount = loader.getFunction<nvmlReturn_t(unsigned int*)>("nvmlDeviceGetCount");

    nvmlDeviceGetName = loader.getFunction<nvmlReturn_t(nvmlDevice_t, char*, unsigned int)>
        ("nvmlDeviceGetName");

    nvmlDeviceGetUtilizationRates = loader.getFunction<nvmlReturn_t(nvmlDevice_t, nvmlUtilization_t*)>
        ("nvmlDeviceGetUtilizationRates");

    nvmlErrorString = loader.getFunction<const char*(nvmlReturn_t)>("nvmlErrorString");

    nvmlDeviceGetSerial = loader.getFunction<nvmlReturn_t(nvmlDevice_t, char*, unsigned int)>
        ("nvmlDeviceGetSerial");

    nvmlSystemGetDriverVersion = loader.getFunction<nvmlReturn_t(char*, unsigned int)>
        ("nvmlSystemGetDriverVersion");

    nvmlDeviceGetEnforcedPowerLimit = loader.getFunction<nvmlReturn_t(nvmlDevice_t, unsigned int*)>
    ("nvmlDeviceGetEnforcedPowerLimit");

    nvmlDeviceGetTemperatureThreshold = 
    loader.getFunction<nvmlReturn_t(nvmlDevice_t, nvmlTemperatureThresholds_t, unsigned int*)>
        ("nvmlDeviceGetTemperatureThreshold");
    
    nvmlDeviceGetNvLinkState = 
    loader.getFunction<nvmlReturn_t(nvmlDevice_t, unsigned int, nvmlEnableState_t *)>
        ("nvmlDeviceGetNvLinkState");

    nvmlDeviceGetNvLinkVersion =
    loader.getFunction<nvmlReturn_t(nvmlDevice_t, unsigned int, unsigned int *)>
        ("nvmlDeviceGetNvLinkVersion");

    nvmlDeviceGetNvLinkCapability =
    loader.getFunction<nvmlReturn_t(nvmlDevice_t, unsigned int, nvmlNvLinkCapability_t, unsigned int *)>
        ("nvmlDeviceGetNvLinkCapability");

    // 调用初始化函数
    nvmlInit();
}

Nvml::~Nvml()
{
    // 调用关闭函数
    nvmlShutdown();
}

void Nvml::handleNVLinkVariant(NVLinkVariant& links, nvDeviceCount deviceIndex)
{
    std::visit([&](auto& links) {
        using T = std::decay_t<decltype(links)>; // 获取实际类型
        if constexpr (std::is_same_v<T, std::array<nvLink, NVML_NVLINK_MAX_LINKS>>) 
        {
            std::size_t thelink = 0;
            for (auto& link : links) 
            {
                bool supported = getNvLinkState(deviceIndex, thelink);
                link.isNvLinkSupported = supported;
                if (link.isNvLinkSupported) 
                {
                    link.link = thelink;
                    link.NvLinkVersion = getNvLinkVersion(deviceIndex, thelink);
                    link.NvLinkCapability = getNvLinkCapability(deviceIndex, thelink);
                }
            }
        } 
        else 
        {
            links = std::string("Unsupport");
        }
    }, links);
}

nvDeviceCount Nvml::getDeviceCount()
{
    nvDeviceCount count;
    const NvReturn& result = nvmlDeviceGetCount(&count);
    nvmlCheckResult(result);

    return count;
}

void Nvml::nvmlCheckResult(const NvReturn& result)
{
    if (result != NVML_SUCCESS) {
        throw NVMLException(std::string("Failed to get device handle: ") + nvmlErrorString(result));
    }
}

NvDeviceHandel Nvml::getDeviceHandle(NvDeviceIndex index)
{
    NvDeviceHandel device;
    NvReturn result = nvmlDeviceGetHandleByIndex(index, &device);
    nvmlCheckResult(result);

    return device;
}

std::string Nvml::getDeviceName(NvDeviceIndex index) {
    // 获取设备句柄
    const NvDeviceHandel& device = getDeviceHandle(index);

    // 使用 NVML_DEVICE_NAME_BUFFER_SIZE 来定义缓冲区大小
    char name[NVML_DEVICE_NAME_BUFFER_SIZE];
    const NvReturn& result = nvmlDeviceGetName(device, name, NVML_DEVICE_NAME_BUFFER_SIZE);
    nvmlCheckResult(result);

    return std::string(name);
}

std::string Nvml::getDeviceSerial(NvDeviceIndex index)
{
    // 获取设备句柄
    const NvDeviceHandel& device = getDeviceHandle(index);

    char serial[NVML_DEVICE_SERIAL_BUFFER_SIZE];
    const NvReturn& result = nvmlDeviceGetSerial(device, serial, NVML_DEVICE_SERIAL_BUFFER_SIZE);
    nvmlCheckResult(result);

    return std::string(serial);
}

std::string Nvml::getDeviceDriverVersion(NvDeviceIndex index)
{
    // 获取设备句柄
    const NvDeviceHandel& device = getDeviceHandle(index);

    char driverVersion[NVML_SYSTEM_DRIVER_VERSION_BUFFER_SIZE];
    const NvReturn& result = nvmlSystemGetDriverVersion(driverVersion, NVML_SYSTEM_DRIVER_VERSION_BUFFER_SIZE);
    nvmlCheckResult(result);

    return std::string(driverVersion);
}

NvPower Nvml::getPowerLimit(NvDeviceIndex index)
{
    // 获取设备句柄
    const NvDeviceHandel& device = getDeviceHandle(index);

    NvPower powerLimit;
    const NvReturn& result = nvmlDeviceGetEnforcedPowerLimit(device, &powerLimit);
    nvmlCheckResult(result);

    return powerLimit;

}

NvDeviceTemperature Nvml::getTemperatureThreshold(NvDeviceIndex index)
{
    // 获取设备句柄
    const NvDeviceHandel& device = getDeviceHandle(index);

    NvDeviceTemperature temperatureThreshold;
    // 默认获取会导致降频率的温度作为温度墙
    const NvReturn& result = nvmlDeviceGetTemperatureThreshold(device, NVML_TEMPERATURE_THRESHOLD_GPU_MAX, &temperatureThreshold);
    nvmlCheckResult(result);

    return temperatureThreshold;
}

bool Nvml::getNvLinkState(NvDeviceIndex index, NVLink link)
{
    // 获取设备句柄
    const NvDeviceHandel& device = getDeviceHandle(index);

    NvEnableState state;
    const NvReturn& result = nvmlDeviceGetNvLinkState(device, link, &state);
    nvmlCheckResult(result);

    return state == NVML_FEATURE_ENABLED;
}

NVLinkVersion Nvml::getNvLinkVersion(NvDeviceIndex index, NVLink link)
{
    // 获取设备句柄
    const NvDeviceHandel& device = getDeviceHandle(index);

    NVLinkVersion version;
    const NvReturn& result = nvmlDeviceGetNvLinkVersion(device, link, &version);
    nvmlCheckResult(result);

    return version;
}

std::string Nvml::getNvLinkCapability(NvDeviceIndex index, NVLink link)
{
    std::string capability = "";

    // 获取设备句柄
    const NvDeviceHandel& device = getDeviceHandle(index);

    for(
        NvLinkCapability i = NVML_NVLINK_CAP_P2P_SUPPORTED;
        i < NVML_NVLINK_CAP_COUNT;
        i = static_cast<NvLinkCapability>(i + 1)
    )
    {
        NvLinkCapabilityResult capResult;
        const NvReturn& result = nvmlDeviceGetNvLinkCapability(device, link, i, &capResult);
        nvmlCheckResult(result);
        switch (i) 
        {
            case NVML_NVLINK_CAP_P2P_SUPPORTED:
                if (capResult) 
                {
                    capability += "[✔] P2P over NVLink is supported\n";
                }
                else
                {
                    capability += "[✘] P2P over NVLink is not supported\n";
                }
                break;
            case NVML_NVLINK_CAP_SYSMEM_ACCESS:
                if (capResult) 
                {
                    capability += "[✔] Access to system memory is supported\n";
                }
                else
                {
                    capability += "[✘] Access to system memory is not supported\n";
                }
                break;
            case NVML_NVLINK_CAP_P2P_ATOMICS:
                if (capResult) 
                {
                    capability += "[✔] P2P Atomics are supported\n";
                }
                else
                {
                    capability += "[✘] P2P Atomics are not supported\n";
                }
                break;
            case NVML_NVLINK_CAP_SYSMEM_ATOMICS:
                if (capResult) 
                {
                    capability += "[✔] System memory Atomics are supported\n";
                }
                else
                {
                    capability += "[✘] System memory Atomics are not supported\n";
                }
                break;
            case NVML_NVLINK_CAP_SLI_BRIDGE:
                if (capResult) 
                {
                    capability += "[✔] SLI Bridge is supported\n";
                }
                else
                {
                    capability += "[✘] SLI Bridge is not supported\n";
                }
                break;
            case NVML_NVLINK_CAP_VALID:
                if (capResult) 
                {
                    capability += "[✔] NVLink is valid\n";
                }
                else
                {
                    capability += "[✘] NVLink is not valid\n";
                }
                break;
            default:
                break;
        }
    }

    return capability;
}

NvUtilization Nvml::getDeviceGetUtilizationRates(NvDeviceIndex index)
{
    // 获取设备句柄
    const NvDeviceHandel& device = getDeviceHandle(index);

    NvUtilization utilization;
    const NvReturn& result = nvmlDeviceGetUtilizationRates(device, &utilization);
    nvmlCheckResult(result);

    return utilization;
}

NvDeviceTemperature Nvml::getDeviceTemperature(NvDeviceIndex index)
{
    // 获取设备句柄
    const NvDeviceHandel& device = getDeviceHandle(index);

    NvDeviceTemperature temperature;
    // 当前似乎只有一种温度传感器类型 NVML_TEMPERATURE_GPU
    const NvReturn& result = nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temperature);
    nvmlCheckResult(result);

    return temperature;
}

NvDeviceFanSpeed Nvml::getDeviceFanSpeed(NvDeviceIndex index, NvFanIndex fanIndex)
{
    // 获取设备句柄
    const NvDeviceHandel& device = getDeviceHandle(index);

    NvDeviceFanSpeed fanSpeed;
    const NvReturn& result = nvmlDeviceGetFanSpeed_v2(device, fanIndex, &fanSpeed);
    nvmlCheckResult(result);

    return fanSpeed;
}

NvFanNum Nvml::getDeviceFanNum(NvDeviceIndex index)
{
    // 获取设备句柄
    const NvDeviceHandel& device = getDeviceHandle(index);

    NvFanNum fanNum;
    const NvReturn& result = nvmlDeviceGetFanSpeed_v2(device, 0, &fanNum);
    nvmlCheckResult(result);

    return fanNum;
}

NvClock Nvml::getDeviceClockInfo(NvDeviceIndex index, NvClockType type)
{
    // 获取设备句柄
    const NvDeviceHandel& device = getDeviceHandle(index);

    NvClock clock;
    const NvReturn& result = nvmlDeviceGetClockInfo(device, type, &clock);
    nvmlCheckResult(result);

    return clock;
}

NvPower Nvml::getDevicePowerUsage(NvDeviceIndex index)
{
    // 获取设备句柄
    const NvDeviceHandel& device = getDeviceHandle(index);

    NvPower powerUsage;
    const NvReturn& result = nvmlDeviceGetPowerUsage(device, &powerUsage);
    nvmlCheckResult(result);

    return powerUsage;
}

} // namespace YSolowork::util