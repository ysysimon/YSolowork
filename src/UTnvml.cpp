#include "UTnvml.h"
#include "nvml.h"
#include <cstdio>
#include <stdexcept>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__linux__)
#include <dlfcn.h>

// 定义函数指针类型
typedef nvmlReturn_t (*NvmlInitFunc)();
typedef const char* (*NvmlErrorStringFunc)(nvmlReturn_t);
typedef nvmlReturn_t (*NvmlDeviceGetHandleByIndexFunc)(unsigned int, nvmlDevice_t*);
typedef nvmlReturn_t (*NvmlDeviceGetNameFunc)(nvmlDevice_t, char*, unsigned int);
typedef nvmlReturn_t (*NvmlDeviceGetUtilizationRatesFunc)(nvmlDevice_t, nvmlUtilization_t*);
typedef nvmlReturn_t (*NvmlShutdownFunc)();

#else
#error "Unsupported platform"
#endif

namespace YSolowork::untility {

std::string test_nvml()
{
    #if defined(_WIN32) || defined(_WIN64)
    // 使用 GetModuleHandleW 检查 nvml.dll 是否已经加载
    HMODULE hNvml = GetModuleHandleW(L"nvml.dll");
    if (!hNvml) {
        // 如果 DLL 不存在，抛出自定义异常
        throw std::runtime_error("nvml.dll is not loaded or does not exist.");
    }

    #endif


    #if defined(__linux__)
    // 使用 dlopen 加载共享库，设置 RTLD_NOLOAD 只检测是否已加载
    void* handle = dlopen("libnvidia-ml.so", RTLD_NOLOAD);
    // 检查是否有错误
    const char* _dlerror = dlerror();
    if (_dlerror || !handle) {
        // 如果共享库不存在，抛出自定义异常
        throw std::runtime_error("libnvidia-ml.so is not loaded or does not exist.");
    }

    // 获取每个函数的符号地址
    NvmlInitFunc nvmlInit = (NvmlInitFunc)dlsym(handle, "nvmlInit_v2");
    NvmlErrorStringFunc nvmlErrorString = (NvmlErrorStringFunc)dlsym(handle, "nvmlErrorString");
    NvmlDeviceGetHandleByIndexFunc nvmlDeviceGetHandleByIndex = (NvmlDeviceGetHandleByIndexFunc)dlsym(handle, "nvmlDeviceGetHandleByIndex_v2");
    NvmlDeviceGetNameFunc nvmlDeviceGetName = (NvmlDeviceGetNameFunc)dlsym(handle, "nvmlDeviceGetName");
    NvmlDeviceGetUtilizationRatesFunc nvmlDeviceGetUtilizationRates = (NvmlDeviceGetUtilizationRatesFunc)dlsym(handle, "nvmlDeviceGetUtilizationRates");
    NvmlShutdownFunc nvmlShutdown = (NvmlShutdownFunc)dlsym(handle, "nvmlShutdown");

    // 检查每个符号的加载是否成功
    const char* error = dlerror();
    if (error != nullptr) {
        dlclose(handle);
        throw std::runtime_error("Failed to load symbol: " + std::string(error));
    }

    #endif

    // 初始化 NVML 库
    nvmlReturn_t result = nvmlInit();
    if (result != NVML_SUCCESS) {
        throw std::runtime_error("Failed to initialize NVML: " + std::string(nvmlErrorString(result)));
    }

    // 获取第一个 GPU 设备的句柄
    nvmlDevice_t device;
    result = nvmlDeviceGetHandleByIndex(0, &device);
    if (result != NVML_SUCCESS) {
        throw std::runtime_error("Failed to get device handle: " + std::string(nvmlErrorString(result)));
    }

    // 获取 GPU 的名称
    char name[NVML_DEVICE_NAME_BUFFER_SIZE];
    result = nvmlDeviceGetName(device, name, NVML_DEVICE_NAME_BUFFER_SIZE);
    if (result != NVML_SUCCESS) {
        throw std::runtime_error("Failed to get device name: " + std::string(nvmlErrorString(result)));
    }

    // 输出 GPU 名称
    // std::cout << "GPU Name: " << name << std::endl;

    // // 获取 GPU 的使用率
    // nvmlUtilization_t utilization;
    // result = nvmlDeviceGetUtilizationRates(device, &utilization);
    // if (result != NVML_SUCCESS) {
    //     throw std::runtime_error("Failed to get GPU utilization: " + std::string(nvmlErrorString(result)));
    // }

    // // 输出 GPU 使用率
    // std::cout << "GPU Utilization: " << utilization.gpu << "%" << std::endl;

    // 关闭 NVML
    nvmlShutdown();

    return name;
}



} // namespace YSolowork::untility