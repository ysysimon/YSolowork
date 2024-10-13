#include "UTfile.h"

#if defined(__linux__)
#include <unistd.h>     // Linux 下获取可执行文件路径
#include <limits.h>     // 用于 PATH_MAX
#include <stdexcept>    // 用于抛出异常
#include <cstring>      // 用于 strerror
#include <cerrno>       // 用于 errno
#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>    // Windows API 头文件，用于 MAX_PATH 和 GetModuleFileName
#endif


namespace YSolowork::util {

std::filesystem::path getExecutablePath() {
    // 根据平台获取可执行文件路径
#if defined(_WIN32) || defined(_WIN64)
    char buffer[MAX_PATH];
    GetModuleFileName(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(buffer).remove_filename();
#elif defined(__linux__)
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        return std::filesystem::path(buffer).remove_filename();
    } else {
        // 错误处理：抛出带有系统错误消息的异常
        throw std::runtime_error(std::string("Cannot retrieve executable path 找不到可执行文件路径 - ") + strerror(errno));
    }
#else
    throw std::runtime_error("Unsupported platform 不支持的系统");
#endif
}

}

