cmake_minimum_required(VERSION 3.6)
project(
    YSolowork 
    VERSION 0.0.0
    DESCRIPTION "YSolowork made by ysysimon" 
    )

# 设置输出目录
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${YSolowork_BINARY_DIR}/build/bin)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${YSolowork_BINARY_DIR}/build/lib)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${YSolowork_BINARY_DIR}/build/lib)
# message("YSolowork_BINARY_DIR: ${YSolowork_BINARY_DIR}")
# message("YSolowork_SOURCE_DIR: ${YSolowork_SOURCE_DIR}")

# 编译器特定的设置
if (MSVC)
    # Visual Studio 中使用 /utf-8 编译选项
    add_compile_options("/utf-8")
    #  Windows.h 定义了 min 和 max 与 std::numeric_limits 发生冲突 (jwt-cpp)
    add_definitions(-DNOMINMAX)
endif()

# 引入 Boost::process 和 Boost::system
find_package(Boost REQUIRED COMPONENTS process system)

# 使用 GLOB 来找到所有的 .cpp 文件
file(GLOB YSolowork_SOURCES "${YSolowork_SOURCE_DIR}/src/*.cpp")
# 添加 YSolowork 库
add_library(YSolowork STATIC ${YSolowork_SOURCES})
# 链接 Boost::process 和 Boost::system
target_link_libraries(YSolowork PRIVATE Boost::process Boost::system)
# _WIN32_WINNT 符号的定义已经转移至 cmake 配置脚本中
# if (MSVC)
#     # Windows 平台需要定义 _WIN32_WINNT=0x0A00 表示 Windows 10
#     # 这是 boost 需要的
#     target_compile_definitions(YSolowork PRIVATE _WIN32_WINNT=0x0A00)
# endif()

# 设置 YSolowork 库编译选项
set_target_properties(YSolowork PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

# 添加选项来控制是否构建 YLineServer 和 YLineWorker
option(BUILD_YLINESERVER "Build YLineServer" ON)
option(BUILD_YLINEWORKER "Build YLineWorker" ON)

# 添加 YLineServer
if (BUILD_YLINESERVER)
    add_subdirectory(YLineServer)
endif()

# 添加 YLineWorker
if (BUILD_YLINEWORKER)
    add_subdirectory(YLineWorker)
endif()