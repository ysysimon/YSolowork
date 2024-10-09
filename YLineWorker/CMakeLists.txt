project(
    YLineWorker 
    VERSION 0.0.1
    DESCRIPTION "YLine is a distributed task scheduling system, YLineWorker is the worker side of YLine." 
    LANGUAGES CXX
)

set(YLineWorker_BUILD_PATH ${YSolowork_BINARY_DIR}/build/bin/YLineWorker)
message("YLineWorker_BUILD_PATH: ${YLineWorker_BUILD_PATH}")

# ---------------------- YLineWorkerUI --------------------------------

# 设置 YLineWorker 源文件
set(YLineWorkerUI_SOURCES
    src/YLineWorkerUI.cpp
    src/ui.cpp
)

# 创建 YLineWorkerUI 可执行文件
add_executable(
    YLineWorkerUI 
    ${YLineWorkerUI_SOURCES}
)

# 设置 YLineWorkerUI 编译选项
target_compile_features(YLineWorkerUI PRIVATE cxx_std_20)
set_target_properties(YLineWorkerUI PROPERTIES
    CXX_STANDARD_REQUIRED YES
    RUNTIME_OUTPUT_DIRECTORY ${YLineWorker_BUILD_PATH}
)

# 添加 YLineWorker 的 include 目录
target_include_directories(
    # add include directory for YLineWorker
    YLineWorkerUI PRIVATE ${YLineWorker_SOURCE_DIR}/inc
    # add include directory for vendor lib
    YLineWorkerUI PRIVATE ${YSolowork_SOURCE_DIR}/vendor
    # add YSolowork include directory
    YLineWorkerUI PRIVATE ${YSolowork_SOURCE_DIR}/src
)

# 添加 FTXUI 库
add_subdirectory(
    ${YSolowork_SOURCE_DIR}/vendor/ftxui
    ${YSolowork_BINARY_DIR}/vendor/ftxui
)
target_link_libraries(
    YLineWorkerUI PRIVATE 
    ftxui::screen
    ftxui::dom
    ftxui::component
)

# 添加 YSolowork 库
target_link_libraries(YLineWorkerUI PRIVATE YSolowork)

# 添加 Drogon 库
target_link_libraries(YLineWorkerUI PRIVATE drogon) # 注意大小写!!! 作为 git 子模块的 drogon 项目名为小写



# ---------------------- YLineWorker Service --------------------------------

# 设置 YLineWorker 源文件
set(YLineWorker_SOURCES
    src/YLineWorker.cpp
    # UT
    src/utils/logger.cpp
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

# 添加 YSolowork 库
target_link_libraries(YLineWorker PRIVATE YSolowork)

# 添加 spdlog 库
find_package(spdlog REQUIRED)
# need to use header only version for building on Windows
target_link_libraries(YLineWorker PRIVATE spdlog::spdlog_header_only)

# 添加 Drogon 库
target_link_libraries(YLineWorker PRIVATE drogon) # 注意大小写!!! 作为 git 子模块的 drogon 项目名为小写

# 设置预编译头文件 for toml.hpp
target_precompile_headers(YLineWorker PRIVATE ${YSolowork_SOURCE_DIR}/vendor/vendor_include/toml.hpp)