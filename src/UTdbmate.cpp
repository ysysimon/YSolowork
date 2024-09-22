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
    fs::path dbmate_path = DBMATE_PATH / DBMATE_BINARY;
    return fs::exists(dbmate_path);
}

// 下载 dbmate
void downloadDbmate(const std::string& DOWNLOAD_URL, const std::string& DOWNLOAD_NAME, const fs::path& DBMATE_PATH) {
    // set download url and download path
    std::string FULL_DOWNLOAD_URL = DOWNLOAD_URL + '/' + DOWNLOAD_NAME;
    fs::path download_path = DBMATE_PATH / DBMATE_BINARY;

    #if defined(_WIN32) || defined(_WIN64)
        const std::string DOWNLOAD_CMD = "powershell -Command \"Invoke-WebRequest -Uri " + FULL_DOWNLOAD_URL + " -OutFile " + download_path.string() + "\"";
    #elif defined(__linux__)
        const std::string DOWNLOAD_CMD = "wget " + FULL_DOWNLOAD_URL + " -O " + download_path.string() + " && chmod +x " + download_path.string();
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