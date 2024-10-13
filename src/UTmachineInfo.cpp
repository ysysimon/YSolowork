#include "UTmachineInfo.h"
#include <stdexcept>

#if defined(__linux__)
#include <unistd.h>  // for gethostname
#include <limits.h>  // for HOST_NAME_MAX
#include <sys/utsname.h> // for struct utsname
#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
typedef LONG(WINAPI* RtlGetVersionFunc)(PRTL_OSVERSIONINFOW);
#endif

#include <thread>
#include <CL/opencl.hpp>

namespace YSolowork::util {

std::string getMachineName()
{
#if defined(__linux__)
    char machineName[HOST_NAME_MAX];
    if (gethostname(machineName, sizeof(machineName)) != 0) {
        throw std::runtime_error("Failed to get Machine Name");
    }
#elif defined(_WIN32) || defined(_WIN64)
    char machineName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(machineName);
    if (!GetComputerNameA(machineName, &size)) {
        throw std::runtime_error("Failed to get Machine Name");
    }
#endif
    return std::string(machineName);
}

unsigned int getCPUcores() noexcept
{
    return std::thread::hardware_concurrency();
}



uint64_t getTotalMemoryBytes() {
#if defined(_WIN32) || defined(_WIN64)
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (GlobalMemoryStatusEx(&statex)) {
        throw std::runtime_error("Failed to get Total Memory Bytes");
    }
    return static_cast<uint64_t>(statex.ullTotalPhys);
#elif defined(__linux__)
    long pages = sysconf(_SC_PHYS_PAGES);  // 获取系统的总页数
    long page_size = sysconf(_SC_PAGE_SIZE);  // 获取每页的字节数
    if (pages == -1 || page_size == -1) {  // 检查是否获取成功
        throw std::runtime_error("Failed to get Total Memory Bytes");
    }
    return static_cast<uint64_t>(pages) * static_cast<uint64_t>(page_size);
#else
    #error "Unsupported platform"
#endif
}

double bytesToGB(uint64_t bytes) {
    return static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0);  // 1 GiB = 2^30 字节
}

double getTotalMemoryGB() {
    return bytesToGB(getTotalMemoryBytes());
}

#if defined(_WIN32) || defined(_WIN64)
// 宽字符转换为多字节字符
std::string WideStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size_needed, NULL, NULL);
    return str;
}
#elif defined(__linux__)
std::unordered_map<std::string, Architecture> archMap = {
    {"x86_64", Architecture::x86_64},
    {"i686", Architecture::x86},
    {"x86", Architecture::x86},
    {"arm64", Architecture::arm64},
    {"aarch64", Architecture::arm64},
    {"arm", Architecture::arm},
};
#endif

SystomInfo getSystomInfo()
{
    SystomInfo systomInfo;
#if defined(_WIN32) || defined(_WIN64)
    systomInfo.os = OS::Windows;

    // 动态加载 RtlGetVersion 函数
    HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
    if (!hMod) {
        throw std::runtime_error("Failed to load ntdll.dll");
    }

    RtlGetVersionFunc RtlGetVersion = (RtlGetVersionFunc)GetProcAddress(hMod, "RtlGetVersion");
    if (!RtlGetVersion) {
        throw std::runtime_error("Failed to find RtlGetVersion function");
    }

    // 使用 RtlGetVersion 获取操作系统版本信息
    RTL_OSVERSIONINFOEXW osviEx = { 0 };
    osviEx.dwOSVersionInfoSize = sizeof(osviEx);
    
    if (RtlGetVersion(reinterpret_cast<PRTL_OSVERSIONINFOW>(&osviEx)) != 0) {
        throw std::runtime_error("Failed to get OS Info");
    }

    // 填充系统信息
    systomInfo.osName = "Windows " + std::to_string(osviEx.dwMajorVersion) + "." + std::to_string(osviEx.dwMinorVersion);
    systomInfo.osRelease = std::to_string(osviEx.dwMajorVersion) + "." + std::to_string(osviEx.dwMinorVersion);
    systomInfo.osVersion = std::to_string(osviEx.dwBuildNumber);

    // 转换 Service Pack 信息 (szCSDVersion) 从宽字符到多字节
    if (wcslen(osviEx.szCSDVersion) > 0) {
        systomInfo.osVersion += " " + WideStringToString(std::wstring(osviEx.szCSDVersion));
    }

    // 获取系统架构信息
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    switch (sysInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            systomInfo.osArchitecture = Architecture::x86_64;
            break;
        case PROCESSOR_ARCHITECTURE_ARM64:
            systomInfo.osArchitecture = Architecture::arm64;
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            systomInfo.osArchitecture = Architecture::x86;
            break;
        case PROCESSOR_ARCHITECTURE_ARM:
            systomInfo.osArchitecture = Architecture::arm;
            break;
        default:
            systomInfo.osArchitecture = Architecture::Unknown;
            break;
    }

#elif defined(__linux__)
    systomInfo.os = OS::Linux;
    struct utsname uts;
    if (uname(&uts) == -1) {
        throw std::runtime_error("Failed to get OS Info");
    }
    systomInfo.osName = uts.sysname;
    systomInfo.osRelease = uts.release;
    systomInfo.osVersion = uts.version;

    std::string machineStr = uts.machine;
    std::transform(machineStr.begin(), machineStr.end(), machineStr.begin(), ::tolower);

    // 查找架构并设置
    auto it = archMap.find(machineStr);
    if (it != archMap.end()) {
        systomInfo.osArchitecture = it->second;
    } else {
        systomInfo.osArchitecture = Architecture::Unknown;
    }

#else
    #error "Unsupported platform"
#endif
    return systomInfo;
}

std::vector<Device> getAllDevices()
{
    std::vector<Device> devices;
    // 获取所有可用的平台
    std::vector<cl::Platform> cl_platforms;
    cl::Platform::get(&cl_platforms);

    // 遍历所有平台
    for (const auto& cl_platform : cl_platforms) 
    {
        
        // 获取平台名称
        std::string platform_Name = cl_platform.getInfo<CL_PLATFORM_NAME>();

        // 获取平台上的所有设备（包括 CPU 和 GPU）
        std::vector<cl::Device> cl_devices;
        cl_platform.getDevices(CL_DEVICE_TYPE_ALL, &cl_devices);

        // 遍历所有设备并打印设备信息
        for (const auto& cl_device : cl_devices) 
        {
            Device device;
            device.platformName = platform_Name;
            device.name = cl_device.getInfo<CL_DEVICE_NAME>();
            device.cores = cl_device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
            device.memoryGB = bytesToGB(cl_device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>());
            switch (cl_device.getInfo<CL_DEVICE_TYPE>()) 
            {
                case CL_DEVICE_TYPE_CPU:
                    device.type = deviceType::CPU;
                    break;
                case CL_DEVICE_TYPE_GPU:
                    device.type = deviceType::GPU;
                    break;
                case CL_DEVICE_TYPE_ACCELERATOR:
                    device.type = deviceType::ACCELERATOR;
                    break;
                default:
                    device.type = deviceType::Unknown;
                    break;
            }

            devices.push_back(device);
        }
    }

    return devices;
}

} // namespace YSolowork::util