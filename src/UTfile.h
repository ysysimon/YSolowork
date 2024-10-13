#ifndef UTFILE_H
#define UTFILE_H

#include <filesystem>

namespace YSolowork::util {

// 函数: 获取可执行文件路径
std::filesystem::path getExecutablePath();

}

#endif // UTFILE_H