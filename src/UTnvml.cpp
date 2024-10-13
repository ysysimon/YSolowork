#include "UTnvml.h"
#include <stdexcept>

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
    nvmlDeviceGetHandleByIndex = loader.getFunction<nvmlReturn_t(unsigned int, nvmlDevice_t*)>("nvmlDeviceGetHandleByIndex_v2");
    nvmlDeviceGetName = loader.getFunction<nvmlReturn_t(nvmlDevice_t, char*, unsigned int)>("nvmlDeviceGetName");
    nvmlDeviceGetUtilizationRates = loader.getFunction<nvmlReturn_t(nvmlDevice_t, nvmlUtilization_t*)>("nvmlDeviceGetUtilizationRates");
    nvmlErrorString = loader.getFunction<const char*(nvmlReturn_t)>("nvmlErrorString");

    // 调用初始化函数
    nvmlInit();
}

Nvml::~Nvml()
{
    // 调用关闭函数
    nvmlShutdown();
}

std::string Nvml::getDeviceName(unsigned int index) {
    nvmlDevice_t device;
    nvmlReturn_t result = nvmlDeviceGetHandleByIndex(index, &device);
    if (result != NVML_SUCCESS) {
        throw std::runtime_error(std::string("Failed to get device handle: ") + nvmlErrorString(result));
    }

    // 使用 NVML_DEVICE_NAME_BUFFER_SIZE 来定义缓冲区大小
    char name[NVML_DEVICE_NAME_BUFFER_SIZE];
    result = nvmlDeviceGetName(device, name, NVML_DEVICE_NAME_BUFFER_SIZE);
    if (result != NVML_SUCCESS) {
        throw std::runtime_error(std::string("Failed to get device name: ") + nvmlErrorString(result));
    }

    return std::string(name);
}

int Nvml::getDeviceUtilization(unsigned int index) {
    nvmlDevice_t device;
    nvmlReturn_t result = nvmlDeviceGetHandleByIndex(index, &device);
    if (result != NVML_SUCCESS) {
        throw std::runtime_error(std::string("Failed to get device handle: ") + nvmlErrorString(result));
    }

    nvmlUtilization_t utilization;
    result = nvmlDeviceGetUtilizationRates(device, &utilization);
    if (result != NVML_SUCCESS) {
        throw std::runtime_error(std::string("Failed to get device utilization: ") + nvmlErrorString(result));
    }

    return utilization.gpu;  // 返回 GPU 利用率
}


} // namespace YSolowork::util