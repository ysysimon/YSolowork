#include "UTappdata.h"
#include <string>

#ifdef _WIN32
#include <stdlib.h>  // for _dupenv_s
#elif __linux__
#include <cstdlib> // for getenv
#endif


namespace YSolowork::util{

std::string getEnvVar(const char* varName) {
    std::string result;
#ifdef _WIN32
    // Use _dupenv_s on Windows
    char* buffer = nullptr;
    size_t len = 0;
    if (_dupenv_s(&buffer, &len, varName) == 0 && buffer != nullptr) {
        result = std::string(buffer);
        free(buffer);
    }
#else
    // Use getenv on other platforms
    const char* envValue = getenv(varName);
    if (envValue) {
        result = std::string(envValue);
    }
#endif
    return result;
}

std::filesystem::path getAppDataDir()
{
    std::filesystem::path appDataDir;

#ifdef _WIN32
    // %APPDATA%：C:\Users\YourUser\AppData\Roaming
    appDataDir = getEnvVar("APPDATA");
#elif __linux__
    // $HOME：/home/YourUser
    appDataDir = getEnvVar("HOME");
#endif

    return appDataDir;

}

} // namespace YSolowork::util