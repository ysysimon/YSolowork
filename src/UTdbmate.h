#ifndef UTDBMATE_H
#define UTDBMATE_H

#include <string>
#include <filesystem>
#include <utility>  // for std::pair

namespace YSolowork::util{

namespace fs = std::filesystem;

extern const std::string DBMATE_BINARY;

// 函数: 检查 dbmate 是否已安装
bool isDbmateInstalled(const fs::path& DBMATE_PATH);

// 函数: 下载 dbmate
void downloadDbmate(const std::string& DOWNLOAD_URL, const std::string& DOWNLOAD_NAME, const fs::path& DBMATE_PATH);

// 函数: 运行 dbmate
std::pair<std::string, int> runDbmate(const fs::path& DBMATE_PATH, const std::string& DBMATE_CMD, const std::string& DB_URL);

}

#endif // UTDBMATE_H