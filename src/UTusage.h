#ifndef UTusage_H
#define UTusage_H

namespace YSolowork::util {

// 结构体: CPU 和 内存使用率
struct UsageInfoCPU {
    double cpuUsage;     // CPU 使用率，百分比
    double memoryUsage;  // 内存使用率，百分比
};

// 函数: 跨平台睡眠
void cross_platform_sleep(int seconds) noexcept;

// 函数: 获取 CPU 和 内存使用率
UsageInfoCPU getUsageInfoCPU() noexcept;

} // namespace YSolowork::util
#endif // UTusage_H