project(
    YLineServer 
    VERSION 0.0.1
    DESCRIPTION "YLine is a distributed task scheduling system, YLineServer is the server side of YLine." 
    LANGUAGES CXX
    )

set(YLineServer_BUILD_PATH ${YSolowork_BINARY_DIR}/build/bin/YLineServer)
message("YLineServer_BUILD_PATH: ${YLineServer_BUILD_PATH}")

# 创建 YLineServer 可执行文件
add_executable(
    YLineServer 
    main.cpp
    logger.cpp
    config.cpp
    app.cpp
    controllers/TestCtrl.cc
)
# 设置 YLineServer 编译选项
set_target_properties(YLineServer PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    RUNTIME_OUTPUT_DIRECTORY ${YLineServer_BUILD_PATH}
)


target_include_directories(
    # add include directory for vendor lib
    YLineServer PRIVATE ${YSolowork_SOURCE_DIR}/vendor
    # add YSolowork include directory
    YLineServer PRIVATE ${YSolowork_SOURCE_DIR}/src
)

# 添加 YSolowork 库
target_link_libraries(YLineServer PRIVATE YSolowork)

# 添加 spdlog 库
find_package(spdlog REQUIRED)
target_link_libraries(YLineServer PRIVATE spdlog::spdlog)

# 添加 Drogon 库
# find_package(Drogon REQUIRED) 在 Windows 上编译导致错误，改用下面的方式
# 添加 Drogon 子模块
add_subdirectory(
    ${YSolowork_SOURCE_DIR}/vendor/drogon
    ${YSolowork_BINARY_DIR}/vendor/drogon
)
target_link_libraries(YLineServer PRIVATE drogon) # 注意大小写!!!

# 设置预编译头文件 for toml.hpp
target_precompile_headers(YLineServer PRIVATE ${YSolowork_SOURCE_DIR}/vendor/vendor_include/toml.hpp)

# 自定义命令：复制配置文件
add_custom_command(
    TARGET YLineServer POST_BUILD  # 在 YLineServer 构建完成后执行
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${YSolowork_SOURCE_DIR}/config_manifest/YLineServer_Config.toml  # 源文件路径
            ${YLineServer_BUILD_PATH}/YLineServer_Config.toml  # 目标路径
)
