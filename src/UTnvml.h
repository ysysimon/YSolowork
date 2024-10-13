#ifndef UTnvml_H
#define UTnvml_H
#include "UTdynlib.h"

#include <string>

#include "nvml.h"

namespace YSolowork::util {

class Nvml {
public:
    explicit Nvml();
    ~Nvml();
    
    // 获取设备名称
    std::string getDeviceName(unsigned int index);
    
    // 获取设备利用率
    int getDeviceUtilization(unsigned int index);

private:
    YSolowork::util::DynamicLibrary loader;

    // NVML 函数的 std::function 封装
    std::function<nvmlReturn_t()> nvmlInit;
    std::function<nvmlReturn_t()> nvmlShutdown;
    std::function<nvmlReturn_t(unsigned int, nvmlDevice_t*)> nvmlDeviceGetHandleByIndex;
    std::function<nvmlReturn_t(nvmlDevice_t, char*, unsigned int)> nvmlDeviceGetName;
    std::function<nvmlReturn_t(nvmlDevice_t, nvmlUtilization_t*)> nvmlDeviceGetUtilizationRates;
    std::function<const char*(nvmlReturn_t)> nvmlErrorString;

};


} // namespace YSolowork::util

#endif // UTnvml_H