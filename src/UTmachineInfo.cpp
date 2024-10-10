#include "UTmachineInfo.h"
#include <stdexcept>

#if defined(__linux__)
#include <unistd.h>  // for gethostname
#include <limits.h>  // for HOST_NAME_MAX
#include <sys/utsname.h> // for struct utsname
#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#include <thread>
#include <fstream>

namespace YSolowork::untility {

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

std::string getCPUInfo()
{
#if defined(_WIN32) || defined(_WIN64)
    HKEY hKey;
    LONG lResult;
    char value[1024];
    DWORD value_length = 1024;

    lResult = RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
                            "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 
                            0, 
                            KEY_READ, 
                            &hKey);
    
    if (lResult != ERROR_SUCCESS) {
        throw std::runtime_error("Failed to open registry key");
    }

    lResult = RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL, (LPBYTE)value, &value_length);

    if (lResult != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        throw std::runtime_error("Failed to query registry key");
    }

    RegCloseKey(hKey);
    return std::string(value);

#elif defined(__linux__)
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (!cpuinfo.is_open()) 
    {
        throw std::runtime_error("Failed to open /proc/cpuinfo");
    }
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.find("model name") != std::string::npos) {
            return line.substr(line.find(":") + 2);  // 获取处理器型号和品牌信息
        }
    }
    return "Unknown";
#else
    #error "Unsupported platform"
#endif
    return "Unknown";
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
    return static_cast<double>(bytes) / 1'000'000'000.0;  // 1 GB = 10^9 字节
}

double getTotalMemoryGB() {
    return bytesToGB(getTotalMemoryBytes());
}

SystomInfo getSystomInfo()
{
    SystomInfo systomInfo;
#if defined(_WIN32) || defined(_WIN64)
    systomInfo.os = OS::Windows;
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if(!GetVersionEx((OSVERSIONINFO*) &osvi))
    {
        throw std::runtime_error("Failed to get OS Info");
    }

    systomInfo.osName = "Windows" + std::to_string(osvi.dwMajorVersion) + "." + std::to_string(osvi.dwMinorVersion);
    systomInfo.osRelease = std::to_string(osvi.dwMajorVersion) + "." + std::to_string(osvi.dwMinorVersion);
    systomInfo.osVersion = std::to_string(osvi.dwBuildNumber);
    if (strlen(osvi.szCSDVersion) > 0) 
    {
            systomInfo.osVersion += " " + std::string(osvi.szCSDVersion);  // 包含 Service Pack 信息
    }

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    switch (sysInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            systomInfo.osArchitecture = "x86_64";
            break;
        case PROCESSOR_ARCHITECTURE_ARM64:
            systomInfo.osArchitecture = "arm64";
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            systomInfo.osArchitecture = "x86";
            break;
        case PROCESSOR_ARCHITECTURE_ARM:
            systomInfo.osArchitecture = "arm";
            break;
        default:
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
    systomInfo.osArchitecture = uts.machine;
#else
    #error "Unsupported platform"
#endif
    return systomInfo;
}

MachineInfo getMachineInfo()
{
    MachineInfo machineInfo;
    machineInfo.cpuCores = getCPUcores();

    try {
        machineInfo.totalMemoryGB = getTotalMemoryGB();
    } catch (const std::exception& e) {
        // 如果获取 totalMemoryGB 失败，继续尝试获取其他信息
        try {
            machineInfo.systomInfo = getSystomInfo();
            machineInfo.machineName = getMachineName();
            machineInfo.cpuInfo = getCPUInfo();
        } catch (const std::exception&) {
            // 如果这里也失败，重新抛出异常
            throw;
        }
        // 抛出之前的异常
        throw;
    }

    try {
        // 在没有异常时，继续获取剩下的系统信息
        machineInfo.systomInfo = getSystomInfo();
        machineInfo.machineName = getMachineName();
        machineInfo.cpuInfo = getCPUInfo();
    } catch (const std::exception&) {
        // 如果发生异常，直接抛出
        throw;
    }
    
    
    
    return machineInfo;
}

} // namespace YSolowork::untility