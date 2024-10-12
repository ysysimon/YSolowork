#include "UTusage.h"

#if defined(__linux__)
#include <unistd.h>   // for sleep
#include <fstream>   // for std::ifstream
#include <sstream>   // for std::istringstream
#include <cstdint>  // for int64_t and uint64_t
#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

namespace YSolowork::untility {

// 函数: 跨平台睡眠
void cross_platform_sleep(int seconds) noexcept
{
    if (seconds < 0) {
        seconds = 0;
    }

#if defined(_WIN32) || defined(_WIN64)
    Sleep(seconds * 1000);  // Windows: Sleep takes milliseconds
#else
    sleep(seconds);         // Linux: sleep takes seconds
#endif
}


#if defined(_WIN32) || defined(_WIN64)
// Windows
struct _CpuUsage {
    FILETIME idle, kernel, user;
};
double calculate_cpu_usage(const _CpuUsage& prev_usage,const _CpuUsage& curr_usage) noexcept
{
    ULARGE_INTEGER idle, kernel, user;

    // 构造当前的系统时间为 quadpart
    idle.LowPart = curr_usage.idle.dwLowDateTime;
    idle.HighPart = curr_usage.idle.dwHighDateTime;
    kernel.LowPart = curr_usage.kernel.dwLowDateTime;
    kernel.HighPart = curr_usage.kernel.dwHighDateTime;
    user.LowPart = curr_usage.user.dwLowDateTime;
    user.HighPart = curr_usage.user.dwHighDateTime;

    ULONGLONG curr_sys_idle = idle.QuadPart;
    ULONGLONG curr_sys_kernel = kernel.QuadPart;
    ULONGLONG curr_sys_user = user.QuadPart;

    // 构造之前的系统时间为 quadpart
    idle.LowPart = prev_usage.idle.dwLowDateTime;
    idle.HighPart = prev_usage.idle.dwHighDateTime;
    kernel.LowPart = prev_usage.kernel.dwLowDateTime;
    kernel.HighPart = prev_usage.kernel.dwHighDateTime;
    user.LowPart = prev_usage.user.dwLowDateTime;
    user.HighPart = prev_usage.user.dwHighDateTime;

    ULONGLONG prev_sys_idle = idle.QuadPart;
    ULONGLONG prev_sys_kernel = kernel.QuadPart;
    ULONGLONG prev_sys_user = user.QuadPart;

    ULONGLONG sys_idle_diff = curr_sys_idle - prev_sys_idle;
    ULONGLONG sys_kernel_diff = curr_sys_kernel - prev_sys_kernel;
    ULONGLONG sys_user_diff = curr_sys_user - prev_sys_user;

    ULONGLONG total_sys = sys_kernel_diff + sys_user_diff;
    ULONGLONG total_idle = sys_idle_diff;

    // 防止除以0的情况
    if (total_sys == 0) {
        // 返回-1表示错误
        return -1.0;
    }

    return (total_sys - total_idle) * 100.0 / total_sys;
}
double get_memory_usage() noexcept
{
    MEMORYSTATUSEX mem_info;
    mem_info.dwLength = sizeof(MEMORYSTATUSEX);
    if (!GlobalMemoryStatusEx(&mem_info))
    {
        return -1.0;  // 返回错误值
    }

    return (mem_info.ullTotalPhys - mem_info.ullAvailPhys) * 100.0 / mem_info.ullTotalPhys;
}
#else
// Linux
struct _CpuUsage {
    uint64_t user, nice, system, idle;
    bool error = false;
};
_CpuUsage get_cpu_usage() noexcept
{
    std::ifstream stat_file("/proc/stat");
    if (!stat_file.is_open()) 
    {
        return {0, 0, 0, 0, true};  // 返回默认值
    }

    std::string line;
    _CpuUsage usage = {0, 0, 0, 0};

    if (std::getline(stat_file, line)) 
    {
        std::istringstream ss(line);
        std::string cpu_label;
        
        // 确保数据解析成功
        if (!(ss >> cpu_label >> usage.user >> usage.nice >> usage.system >> usage.idle)) {
            return {0, 0, 0, 0, true};  // 返回默认值
        }
    } else {
        return {0, 0, 0, 0, true};  // 返回默认值
    }

    return usage;
}
double calculate_cpu_usage(const _CpuUsage& prev, const _CpuUsage& curr) noexcept
{
    uint64_t prev_total = prev.user + prev.nice + prev.system + prev.idle;
    uint64_t total = curr.user + curr.nice + curr.system + curr.idle;
    uint64_t total_diff = total - prev_total;
    uint64_t idle_diff = curr.idle - prev.idle;

    if (total_diff == 0) {
        return 0.0; // 返回默认值
    }

    return (total_diff - idle_diff) * 100.0 / total_diff;
}

double get_memory_usage() noexcept
{
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    uint64_t total_memory = 0, available_memory = 0;

    if (!meminfo.is_open()) {
        return -1.0;  // 返回默认值 -1 表示读取失败
    }

    while (std::getline(meminfo, line)) {
        if (line.find("MemTotal:") == 0) {
            if (sscanf(line.c_str(), "MemTotal: %ld kB", &total_memory) != 1) {
                return -1.0;  // 返回默认值
            }
        } else if (line.find("MemAvailable:") == 0) {
            if (sscanf(line.c_str(), "MemAvailable: %ld kB", &available_memory) != 1) {
                return -1.0;  // 返回默认值
            }
        }
    }

    if (total_memory == 0) {
        return -1.0;  // 返回错误值
    }

    uint64_t used_memory = total_memory - available_memory;
    return (used_memory * 100.0) / total_memory;
}
#endif

UsageInfoCPU getUsageInfoCPU() noexcept
{
    UsageInfoCPU usageInfoCPU = {0.0, 0.0};

#if defined(_WIN32) || defined(_WIN64)
    // 获取 CPU 使用率
    // 定义 FILETIME 结构体
    _CpuUsage prev_usage, curr_usage;

    // 获取初始的系统时间
    if (!GetSystemTimes(&prev_usage.idle, &prev_usage.kernel, &prev_usage.user)) {
        // GetSystemTimes 调用失败，返回 -1
        usageInfoCPU.cpuUsage = -1.0;
    }
    cross_platform_sleep(1);  // 等待1秒
    // 获取系统时间在等待后的状态
    if (!GetSystemTimes(&curr_usage.idle, &curr_usage.kernel, &curr_usage.user)) {
        // GetSystemTimes 调用失败，返回 -1
        usageInfoCPU.cpuUsage = -1.0;
    }
    else 
    {
        usageInfoCPU.cpuUsage = calculate_cpu_usage(prev_usage, curr_usage);
    }

    // 获取内存使用率
    usageInfoCPU.memoryUsage = get_memory_usage();
#else
    // 获取 CPU 使用率
    _CpuUsage prev_usage = get_cpu_usage();
    cross_platform_sleep(1);  // 等待1秒
    _CpuUsage curr_usage = get_cpu_usage();
    if (prev_usage.error || curr_usage.error) 
    {
        usageInfoCPU.cpuUsage = -1.0;  // 返回错误值
    }
    else 
    {
        usageInfoCPU.cpuUsage = calculate_cpu_usage(prev_usage, curr_usage);
    }
    
    // 获取内存使用率
    usageInfoCPU.memoryUsage = get_memory_usage();
#endif
    return usageInfoCPU;
}

} // namespace YSolowork::untility