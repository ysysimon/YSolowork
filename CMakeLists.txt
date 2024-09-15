cmake_minimum_required(VERSION 3.6)
project(
    YSoloworkServer 
    VERSION 0.0.1
    DESCRIPTION "YSoloworkServer is a server for YSolowork" 
    LANGUAGES CXX
    )

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${YSoloworkServer_BINARY_DIR}/build/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${YSoloworkServer_BINARY_DIR}/build/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${YSoloworkServer_BINARY_DIR}/build/lib)
message("YSoloworkServer_BINARY_DIR: ${YSoloworkServer_BINARY_DIR}")
message("YSoloworkServer_SOURCE_DIR: ${YSoloworkServer_SOURCE_DIR}")

# 添加模块
add_subdirectory(YLine)