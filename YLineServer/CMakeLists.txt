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
    # Controller
    src/controllers/YLineServer_HealthCheckCtrl.cc
    src/controllers/YLineServer_UserCtrl.cc
    src/controllers/YLineServer_WorkerCtrl.cc
    # Middleware
    src/middlewares/YLineServer_CORSMid.cc
    # UT
    src/utils/logger.cpp
    src/utils/config.cpp
    src/utils/api.cpp
    src/utils/passwd.cpp
    src/utils/jwt.cpp
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
    # add include directory for vendor lib
    YLineServer PRIVATE ${YSolowork_SOURCE_DIR}/vendor
    # add YSolowork include directory
    YLineServer PRIVATE ${YSolowork_SOURCE_DIR}/src
)

# 添加 YSolowork 库
target_link_libraries(YLineServer PRIVATE YSolowork)

# 添加 spdlog 库
find_package(spdlog REQUIRED)
# need to use header only version for building on Windows
target_link_libraries(YLineServer PRIVATE spdlog::spdlog_header_only)

# 添加 Drogon 库
target_link_libraries(YLineServer PRIVATE drogon) # 注意大小写!!! 作为 git 子模块的 drogon 项目名为小写

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
    target_link_libraries(YLineServer PRIVATE Boost::uuid)
else()
    find_package(Boost REQUIRED)  # 仅查找 Boost，跳过 uuid 组件检查
    target_include_directories(YLineServer PRIVATE ${Boost_INCLUDE_DIRS})
endif()

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

# 自定义命令：复制数据库迁移文件
# file(GLOB DB_MIGRATIONS_FILES "${YLineServer_SOURCE_DIR}/db/migrations/*.sql")
# foreach(DB_MIGRATIONS_FILE ${DB_MIGRATIONS_FILES})
#     # 获取文件名
#     get_filename_component(DB_MIGRATIONS_FILE_NAME ${DB_MIGRATIONS_FILE} NAME)
#     add_custom_command(
#         TARGET YLineServer POST_BUILD  # 在 YLineServer 构建完成后执行
#         # 创建目录, 放在 foreach 里是安全的，只会创建一次，不会覆盖和报错
#         COMMAND ${CMAKE_COMMAND} -E make_directory ${YLineServer_BUILD_PATH}/db/migrations
#         COMMAND ${CMAKE_COMMAND} -E copy_if_different
#                 ${DB_MIGRATIONS_FILE}  # 数据库迁移文件源路径
#                 ${YLineServer_BUILD_PATH}/db/migrations/${DB_MIGRATIONS_FILE_NAME}  # 目标路径
#         COMMENT "Copying database migration files to build directory"
#     )
# endforeach()
# 用 copy_directory 代替上面的 foreach, 保持迁移文件的一致性
add_custom_command(
    TARGET YLineServer POST_BUILD  # 在 YLineServer 构建完成后执行
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${YLineServer_SOURCE_DIR}/db  # 数据库迁移文件源路径
            ${YLineServer_BUILD_PATH}/db  # 目标路径
    COMMENT "Copying database migration files to build directory"
)