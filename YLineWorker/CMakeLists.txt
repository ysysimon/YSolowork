project(
    YLineWorker 
    VERSION 0.0.1
    DESCRIPTION "YLine is a distributed task scheduling system, YLineWorker is the worker side of YLine." 
    LANGUAGES CXX
)

set(YLineWorker_BUILD_PATH ${YSolowork_BINARY_DIR}/build/bin/YLineWorker)
message("YLineWorker_BUILD_PATH: ${YLineWorker_BUILD_PATH}")

# 设置 YLineWorker 源文件
set(YLineWorker_SOURCES
    src/main.cpp
)

# 创建 YLineWorker 可执行文件
add_executable(
    YLineWorker 
    ${YLineWorker_SOURCES}
)
# 设置 YLineWorker 编译选项
target_compile_features(YLineWorker PRIVATE cxx_std_20)
set_target_properties(YLineWorker PROPERTIES
    CXX_STANDARD_REQUIRED YES
    RUNTIME_OUTPUT_DIRECTORY ${YLineWorker_BUILD_PATH}
)

# 添加 YLineWorker 的 include 目录
target_include_directories(
    # add include directory for YLineWorker
    YLineWorker PRIVATE ${YLineWorker_SOURCE_DIR}/inc
    # add include directory for vendor lib
    YLineWorker PRIVATE ${YSolowork_SOURCE_DIR}/vendor
    # add YSolowork include directory
    YLineWorker PRIVATE ${YSolowork_SOURCE_DIR}/src
)

# 添加 FTXUI 库
add_subdirectory(
    ${YSolowork_SOURCE_DIR}/vendor/ftxui
    ${YSolowork_BINARY_DIR}/vendor/ftxui
)
target_link_libraries(
    YLineWorker PRIVATE 
    ftxui::screen
    ftxui::dom
    ftxui::component
) 

# 添加 YSolowork 库
target_link_libraries(YLineWorker PRIVATE YSolowork)