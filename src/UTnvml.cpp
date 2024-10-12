#include "UTnvml.h"
#include "nvml.h"
#include <stdexcept>

#include <windows.h>

namespace YSolowork::untility {

std::string test_nvml()
{
    // 使用 GetModuleHandleW 检查 nvml.dll 是否已经加载
    HMODULE hNvml = GetModuleHandleW(L"nvml.dll");

    if (hNvml == NULL) {
        // 如果 DLL 不存在，抛出自定义异常
        throw std::runtime_error("nvml.dll is not loaded or does not exist.");
    }

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