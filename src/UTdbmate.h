#ifndef UTDBMATE_H
#define UTDBMATE_H

#include <string>
#include <filesystem>

namespace YSolowork::untility{

namespace fs = std::filesystem;

extern const std::string DBMATE_BINARY;

// 函数: 检查 dbmate 是否已安装
bool isDbmateInstalled(const fs::path& DBMATE_PATH);

// 函数: 下载 dbmate
void downloadDbmate(const std::string& DOWNLOAD_URL, const std::string& DOWNLOAD_NAME, const fs::path& DBMATE_PATH);

// 函数: 运行 dbmate
void runDbmate(const fs::path& DBMATE_PATH, const std::string& DBMATE_CMD);

}

#endif // UTDBMATE_H