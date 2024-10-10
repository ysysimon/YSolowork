#include "UTconsoleUTF8.h"
#include <stdexcept>

#if defined(__linux__)
#include <locale.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif


namespace YSolowork::untility{

void setConsoleUTF8() {
#if defined(_WIN32) || defined(_WIN64)
    // Windows 平台：设置控制台为 UTF-8 编码
    if (SetConsoleOutputCP(CP_UTF8) == 0) {
        std::runtime_error("Failed to set output code page");
    }
    if (SetConsoleCP(CP_UTF8) == 0) {
        std::runtime_error("Failed to set input code page");
    }
    // 将标准输出和标准输入设置为 UTF-8 模式 会影响 FTXUI, 只设置上面的代码页应该已经足够了
    // _setmode(_fileno(stdout), _O_U8TEXT);
    // _setmode(_fileno(stdin), _O_U8TEXT);
#elif defined(__linux__)
    // POSIX 系统 (Linux, macOS)：设置终端区域为 UTF-8
    if (!setlocale(LC_ALL, "en_US.UTF-8")) {
        std::runtime_error("Failed to set locale to en_US.UTF-8");
    }
#endif
}
    





} // namespace YSolowork::untility