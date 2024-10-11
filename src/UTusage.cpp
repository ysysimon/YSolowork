#include "UTusage.h"

#if defined(__linux__)
#include <unistd.h>   // for sleep
#include <fstream>   // for std::ifstream
#include <sstream>   // for std::istringstream
#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#include <cstdint>  // for int64_t and uint64_t

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


// 计算 CPU 使用率
#if defined(_WIN32) || defined(_WIN64)
double calculate_cpu_usage(FILETIME idle_time, FILETIME kernel_time, FILETIME user_time) {
    ULARGE_INTEGER idle, kernel, user;
    idle.LowPart = idle_time.dwLowDateTime;
    idle.HighPart = idle_time.dwHighDateTime;
    kernel.LowPart = kernel_time.dwLowDateTime;
    kernel.HighPart = kernel_time.dwHighDateTime;
    user.LowPart = user_time.dwLowDateTime;
    user.HighPart = user_time.dwHighDateTime;

    ULONGLONG sys_idle = idle.QuadPart;
    ULONGLONG sys_kernel = kernel.QuadPart;
    ULONGLONG sys_user = user.QuadPart;

    static ULONGLONG prev_sys_idle = 0, prev_sys_kernel = 0, prev_sys_user = 0;

    ULONGLONG sys_idle_diff = sys_idle - prev_sys_idle;
    ULONGLONG sys_kernel_diff = sys_kernel - prev_sys_kernel;
    ULONGLONG sys_user_diff = sys_user - prev_sys_user;

    prev_sys_idle = sys_idle;
    prev_sys_kernel = sys_kernel;
    prev_sys_user = sys_user;

    ULONGLONG total_sys = sys_kernel_diff + sys_user_diff;
    ULONGLONG total_idle = sys_idle_diff;

    return (total_sys - total_idle) * 100.0 / total_sys;
}
#else
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
    FILETIME idle_time, kernel_time, user_time;
    GetSystemTimes(&idle_time, &kernel_time, &user_time);
    cross_platform_sleep(1)  // 等待1秒
    FILETIME idle_time_after, kernel_time_after, user_time_after;
    GetSystemTimes(&idle_time_after, &kernel_time_after, &user_time_after);
    usageInfoCPU.cpuUsage = calculate_cpu_usage(idle_time_after, kernel_time_after, user_time_after);

    // 获取内存使用率
    MEMORYSTATUSEX mem_info;
    mem_info.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&mem_info);
    usageInfoCPU.memoryUsage = (mem_info.ullTotalPhys - mem_info.ullAvailPhys) * 100.0 / mem_info.ullTotalPhys;
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