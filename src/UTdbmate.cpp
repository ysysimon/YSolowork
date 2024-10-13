#include "UTdbmate.h"



#include <cstdlib>
#include <stdexcept>
#include <format>
#include <iostream>

#include <boost/process.hpp>

namespace YSolowork::util {

#if defined(_WIN32) || defined(_WIN64)
    const std::string DBMATE_BINARY = "dbmate.exe";
    #include <boost/process/io.hpp>
    #include <boost/process/pipe.hpp>
    // const std::string DBMATE_INSTALL_URL = "https://github.com/amacneil/dbmate/releases/download/v1.15.0/dbmate-windows-amd64.exe";
    // const std::string DOWNLOAD_CMD = "powershell -Command \"Invoke-WebRequest -Uri " + DBMATE_INSTALL_URL + " -OutFile dbmate.exe\"";
#elif defined(__linux__)
    const std::string DBMATE_BINARY = "dbmate";
    #include <boost/process/v1/io.hpp>
    #include <boost/process/v1/pipe.hpp>
    // const std::string DBMATE_INSTALL_URL = "https://github.com/amacneil/dbmate/releases/download/v1.15.0/dbmate-linux-amd64";
    // const std::string DOWNLOAD_CMD = "wget " + DBMATE_INSTALL_URL + " -O dbmate && chmod +x dbmate";
#endif

// 检查文件是否存在
bool isDbmateInstalled(const fs::path& DBMATE_PATH) {
    const fs::path& dbmate_path = DBMATE_PATH / DBMATE_BINARY;
    return fs::exists(dbmate_path);
}

// 运行 dbmate
std::pair<std::string, int> runDbmate(const fs::path& DBMATE_PATH, const std::string& DBMATE_CMD, const std::string& DB_URL) {
    namespace bp = boost::process;

    // set dbmate path
    const fs::path& dbmate_path = DBMATE_PATH / DBMATE_BINARY;

    // set environment variable
    const bp::environment& env = boost::this_process::environment(); // get current env (这是需要的，否则无法解析 host)
    bp::environment _env = env; // copy env 只影响子进程
    _env["DATABASE_URL"] = DB_URL;

    // set pipe stream
    bp::ipstream out_stream;
    bp::ipstream err_stream;

    // set result log
    std::ostringstream result_log; 
    result_log << std::endl;

    int exit_code = -1;
    // run dbmate migrate
    try {
        // 注意 bp::std_out 和 bd::std_err 在 Linux 上不能同时输出到同一个流，在 Windows 却可以，所以需要进行分离
        bp::child c(dbmate_path.string(), DBMATE_CMD, bp::std_out > out_stream, bp::std_err > err_stream, bp::env = _env);

        // 提前开始读取输出
        std::string line;
        // result_log << "Out Stream 输出流: " << std::endl;
        while (out_stream && std::getline(out_stream, line)) {
            result_log << line << std::endl;
        }

        // result_log << "Error Stream 错误流: " << std::endl;

        while (err_stream && std::getline(err_stream, line)) {
            result_log << line << std::endl;
        }

        c.wait();

        // 输出退出码到日志
        exit_code = c.exit_code();
        result_log << std::endl << "DBMATE Exit code 退出代码: " << exit_code << std::endl;

    } catch (const std::exception& e) {
        // 用于抛出子进程异常
        throw std::runtime_error(e.what());
    }

    return {result_log.str(), exit_code};
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

} // namespace YSolowork::util