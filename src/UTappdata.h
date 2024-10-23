#ifndef UTappdata_H
#define UTappdata_H

#include <filesystem>

namespace YSolowork::util{

// 函数: 获取本地应用程序数据目录
std::filesystem::path getAppDataDir();

} // namespace YSolowork::util

#endif // UTappdata_H