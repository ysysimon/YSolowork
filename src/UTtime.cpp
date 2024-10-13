#include "UTtime.h"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace YSolowork::util {

std::string getCurrentTimestampStr() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_time), "%Y-%m-%d_%H-%M-%S");
    return ss.str();
}


}