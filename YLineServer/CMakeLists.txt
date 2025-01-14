project(
    YLineServer 
    VERSION 0.0.1
    DESCRIPTION "YLine is a distributed task scheduling system, YLineServer is the server side of YLine." 
    LANGUAGES CXX
)

set(YLineServer_BUILD_PATH ${YSolowork_BINARY_DIR}/build/bin/YLineServer)
message("YLineServer_BUILD_PATH: ${YLineServer_BUILD_PATH}")

# 设置 YLineServer 源文件
set(YLineServer_SOURCES
    src/main.cpp
    src/app.cpp
    src/database.cpp
    # ORM
    src/models/Users.cc
    src/models/Workers.cc
    src/models/Jobs.cc
    src/models/Tasks.cc
    # Controller
    src/controllers/YLineServer_HealthCheckCtrl.cc
    src/controllers/YLineServer_UserCtrl.cc
    src/controllers/YLineServer_WorkCtrl.cc
    src/controllers/YLineServer_WorkerCtrl.cc
    src/controllers/WorkerRegister.cpp
    src/controllers/YLineServer_WorkerStatusCtrl.cc
    src/controllers/YLineServer_JobStatusCtrl.cc
    src/controllers/YLineServer_JobCtrl.cc
    # Middleware
    src/middlewares/YLineServer_CORSMid.cc
    # Filter
    src/filters/YLineServer_LoginFilter.cc
    src/filters/YLineServer_AdminFilter.cc
    # UT
    src/utils/logger.cpp
    src/utils/config.cpp
    src/utils/api.cpp
    src/utils/server.cpp
    src/utils/passwd.cpp
    src/utils/jwt.cpp
    # AMQP
    src/AMQP/TrantorHandler.cpp
    src/AMQP/AMQPconnectionPool.cpp
    # task
    src/task/consumer.cpp
    # components
    src/components/worker.cpp
)

# 创建 YLineServer 可执行文件
add_executable(
    YLineServer 
    ${YLineServer_SOURCES}
)
# 设置 YLineServer 编译选项
target_compile_features(YLineServer PRIVATE cxx_std_20)
set_target_properties(YLineServer PROPERTIES
    CXX_STANDARD_REQUIRED YES
    RUNTIME_OUTPUT_DIRECTORY ${YLineServer_BUILD_PATH}
)

# 添加 YLineServer 的 include 目录
target_include_directories(
    # add include directory for YLineServer
    YLineServer PRIVATE ${YLineServer_SOURCE_DIR}/inc
)

# 添加 YSolowork 库
target_link_libraries(YLineServer PRIVATE YSolowork)

# 添加 spdlog 库
find_package(spdlog REQUIRED)
# need to use header only version for building on Windows
target_link_libraries(YLineServer PRIVATE spdlog::spdlog_header_only)

# 添加 jwt++ 库
add_subdirectory(
    ${YSolowork_SOURCE_DIR}/vendor/jwt-cpp
    ${YSolowork_BINARY_DIR}/vendor/jwt-cpp
)
target_link_libraries(YLineServer PRIVATE jwt-cpp)

# 添加 botan 库
find_package(botan REQUIRED)
target_link_libraries(YLineServer PRIVATE Botan::Botan)

# 添加 Boost::uuid 库
if (MSVC)
    find_package(Boost REQUIRED COMPONENTS uuid)
    target_link_libraries(
        YLineServer 
        PRIVATE 
        Boost::uuid
        bcrypt.lib # boost::uuid 需要 bcrypt.lib 在 win 上作为随机数生成器
    )
else()
    find_package(Boost REQUIRED)  # 仅查找 Boost，跳过 uuid 组件检查
    target_include_directories(YLineServer PRIVATE ${Boost_INCLUDE_DIRS})
endif()

# 添加 Boost::graph 库
find_package(Boost REQUIRED COMPONENTS graph)
target_link_libraries(YLineServer PRIVATE Boost::graph)

# 添加 entt 库
add_subdirectory(
    ${YSolowork_SOURCE_DIR}/vendor/entt
    ${YSolowork_BINARY_DIR}/vendor/entt
)
target_link_libraries(YLineServer PRIVATE EnTT::EnTT)
target_compile_definitions(YLineServer PRIVATE ENTT_USE_ATOMIC)
target_compile_definitions(YLineServer PRIVATE ENTT_ID_TYPE=std::uint32_t)

# 添加 tl::expected 库
add_subdirectory(
    ${YSolowork_SOURCE_DIR}/vendor/tl-expected
    ${YSolowork_BINARY_DIR}/vendor/tl-expected
)
target_link_libraries(YLineServer PRIVATE tl::expected)

# 添加 AMQP-CPP 库
add_subdirectory(
    ${YSolowork_SOURCE_DIR}/vendor/amqpcpp
    ${YSolowork_BINARY_DIR}/vendor/amqpcpp
)
target_link_libraries(YLineServer PRIVATE amqpcpp)

# 设置预编译头文件 for toml.hpp
target_precompile_headers(YLineServer PRIVATE ${YSolowork_SOURCE_DIR}/vendor/vendor_include/toml.hpp)

# 自定义命令：复制配置文件
add_custom_command(
    TARGET YLineServer POST_BUILD  # 在 YLineServer 构建完成后执行
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${YSolowork_SOURCE_DIR}/config_manifest/YLineServer_Config.toml  # 配置文件源路径
            ${YLineServer_BUILD_PATH}/YLineServer_Config.toml  # 目标路径
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${YLineServer_SOURCE_DIR}/docker-compose.yml  # docker-compose.yml 源路径
            ${YLineServer_BUILD_PATH}/docker-compose.yml  # 目标路径
    COMMENT "Copying config files to build directory"
)

# 用 copy_directory 复制数据库迁移脚本, 保持迁移文件的一致性
add_custom_command(
    TARGET YLineServer POST_BUILD  # 在 YLineServer 构建完成后执行
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${YLineServer_SOURCE_DIR}/db  # 数据库迁移文件源路径
            ${YLineServer_BUILD_PATH}/db  # 目标路径
    COMMENT "Copying database migration files to build directory"
)