#ifndef UTmachineInfo
#define UTmachineInfo

#include "string"
#include <string>

#include <cstdint>  // For uint64_t

namespace YSolowork::untility {

// 函数: 获取机器名
std::string getMachineName();

// 函数: 获取 CPU 核心数
unsigned int getCPUcores() noexcept;

// 函数: 获取 CPU 信息
std::string getCPUInfo();

// 函数: 获取总内存大小
uint64_t getTotalMemoryBytes();

// 函数: 字节转换为GB
double bytesToGB(uint64_t bytes);

// 函数: 获取总内存大小(GB)
double getTotalMemoryGB();

// 枚举: 操作系统
enum class OS {
    Windows,
    Linux,
    MacOS,
    Unknown
};

// 结构体: 系统信息
struct SystomInfo {
    OS os = OS::Unknown;
    std::string osName = "Unknown";
    std::string osRelease = "Unknown";
    std::string osVersion = "Unknown";
    std::string osArchitecture = "Unknown";
};

// 函数: 获取系统信息
SystomInfo getSystomInfo();

// 结构体: 机器信息
struct MachineInfo {
    SystomInfo systomInfo;
    std::string machineName = "Unknown";
    std::string cpuInfo = "Unknown";
    unsigned int cpuCores = 0;
    double totalMemoryGB = 0.0;
};

// 函数: 获取机器信息
MachineInfo getMachineInfo();

} // namespace YSolowork::untility



#endif // UTmachineInfo