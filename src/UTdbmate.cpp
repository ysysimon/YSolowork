#include "UTdbmate.h"

#include <cstdlib>
#include <stdexcept>
#include <format>


namespace YSolowork::untility {

#if defined(_WIN32) || defined(_WIN64)
    const std::string DBMATE_BINARY = "dbmate.exe";
    // const std::string DBMATE_INSTALL_URL = "https://github.com/amacneil/dbmate/releases/download/v1.15.0/dbmate-windows-amd64.exe";
    // const std::string DOWNLOAD_CMD = "powershell -Command \"Invoke-WebRequest -Uri " + DBMATE_INSTALL_URL + " -OutFile dbmate.exe\"";
#elif defined(__linux__)
    const std::string DBMATE_BINARY = "dbmate";
    // const std::string DBMATE_INSTALL_URL = "https://github.com/amacneil/dbmate/releases/download/v1.15.0/dbmate-linux-amd64";
    // const std::string DOWNLOAD_CMD = "wget " + DBMATE_INSTALL_URL + " -O dbmate && chmod +x dbmate";
#endif

// 检查文件是否存在
bool isDbmateInstalled(const fs::path& DBMATE_PATH) {
    const fs::path& dbmate_path = DBMATE_PATH / DBMATE_BINARY;
    return fs::exists(dbmate_path);
}

// 运行 dbmate
void runDbmate(const fs::path& DBMATE_PATH, const std::string& DBMATE_CMD) {
    // set dbmate path
    const fs::path& dbmate_path = DBMATE_PATH / DBMATE_BINARY;

    // run dbmate migrate
    try {
        system(std::format("{} {}", dbmate_path.string(), DBMATE_CMD).c_str());
    } catch (const std::exception& e) {
        throw std::runtime_error(e.what());
    }
}

// 下载 dbmate
void downloadDbmate(const std::string& DOWNLOAD_URL, const std::string& DOWNLOAD_NAME, const fs::path& DBMATE_PATH) {
    // set download url and download path
    std::string FULL_DOWNLOAD_URL = DOWNLOAD_URL + '/' + DOWNLOAD_NAME;
    fs::path download_path = DBMATE_PATH / DBMATE_BINARY;

    #if defined(_WIN32) || defined(_WIN64)
        const std::string DOWNLOAD_CMD = std::format(
            "powershell -Command \"Invoke-WebRequest -Uri {} -OutFile {}\"",
            FULL_DOWNLOAD_URL, 
            download_path.string()
        );
    #elif defined(__linux__)
        const std::string DOWNLOAD_CMD = std::format(
            "wget {} -O {} && chmod +x {}",
            FULL_DOWNLOAD_URL, 
            download_path.string(), 
            download_path.string()
        );
    #else
        throw std::runtime_error("Unsupported platform 不支持的系统");
    #endif

    // download dbmate
    try {
        system(DOWNLOAD_CMD.c_str());
    } catch (const std::exception& e) {
        std::runtime_error(e.what());
    }
}

} // namespace YSolowork::untility