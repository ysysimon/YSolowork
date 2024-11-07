#ifndef UTmachineInfo_H
#define UTmachineInfo_H

#include "string"
#include <string>

#include <cstdint>  // For uint64_t
#include <vector>

namespace YSolowork::util {

// 枚举: 操作系统
enum class OS {
    Windows,
    Linux,
    MacOS,
    Unknown
};

// 枚举: 架构
enum class Architecture {
    x86,
    x86_64,
    arm,
    arm64,
    Unknown
};

// 枚举: 设备类型
enum class deviceType {
    CPU,
    GPU,
    ACCELERATOR,
    Unknown
};

// 结构体: 设备
struct Device {
    deviceType type = deviceType::Unknown;
    std::string platformName = "Unknown";
    std::string name = "Unknown";
    unsigned int cores = 0;
    double memoryGB = 0.0;
};

// 结构体: 系统信息
struct SystomInfo {
    OS os = OS::Unknown;
    std::string osName = "Unknown";
    std::string osRelease = "Unknown";
    std::string osVersion = "Unknown";
    Architecture osArchitecture = Architecture::Unknown;
};

// 结构体: 机器信息
struct MachineInfo {
    SystomInfo systomInfo;
    std::string machineName = "Unknown";
    std::vector<Device> devices;
};

// 函数: 获取机器名
std::string getMachineName();

// 函数: 获取 CPU 核心数
unsigned int getCPUcores() noexcept;

// 函数: 获取所有 Device
std::vector<Device> getAllDevices();

// 函数: 获取总内存大小
uint64_t getTotalMemoryBytes();

// 函数: 字节转换为GB
double bytesToGB(uint64_t bytes);

// 函数: 获取总内存大小(GB)
double getTotalMemoryGB();

// 函数: 获取系统信息
SystomInfo getSystomInfo();

// 函数: 获取机器信息
MachineInfo getMachineInfo();

} // namespace YSolowork::util
#endif // UTmachineInfo_H