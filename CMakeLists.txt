cmake_minimum_required(VERSION 3.6)
project(
    YSolowork 
    VERSION 0.0.0
    DESCRIPTION "YSolowork made by ysysimon" 
    )

# 设置输出目录 以防被更大的项目包含 先注释了
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${YSolowork_BINARY_DIR}/build/bin)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${YSolowork_BINARY_DIR}/build/lib)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${YSolowork_BINARY_DIR}/build/lib)
# message("YSolowork_BINARY_DIR: ${YSolowork_BINARY_DIR}")
# message("YSolowork_SOURCE_DIR: ${YSolowork_SOURCE_DIR}")

# 编译器特定的设置
if (MSVC)
    # Visual Studio 中使用 /utf-8 编译选项
    add_compile_options("/utf-8")
endif()

# 使用 GLOB 来找到所有的 .cpp 文件
file(GLOB YSolowork_SOURCES "${YSolowork_SOURCE_DIR}/src/*.cpp")
# 添加 YSolowork 库
add_library(YSolowork STATIC ${YSolowork_SOURCES})

# 设置 YSolowork 库编译选项
set_target_properties(YSolowork PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

# 添加模块
add_subdirectory(YLineServer)