cmake_minimum_required(VERSION 3.15)
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
    # 强制子项目继承 MSVC 运行时库设置
    set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
    # 仅在 Release 模式下设置 MSVC 静态运行时库
    # Only set MSVC static runtime library in Release mode
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        # 单配置生成器（如 Makefile 或 Ninja）
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
    elseif(CMAKE_CONFIGURATION_TYPES)
        # 多配置生成器（如 Visual Studio）
        set(CMAKE_MSVC_RUNTIME_LIBRARY "$<$<CONFIG:Release>:MultiThreaded>")
    endif()

    # for debug MSVC runtim static linking of debug build, but it's not working
    # set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
endif()

# 引入 Boost::process 和 Boost::system
find_package(Boost REQUIRED COMPONENTS process system)

# 使用 GLOB 来找到所有的 .cpp 文件
file(GLOB YSolowork_SOURCES "${YSolowork_SOURCE_DIR}/src/*.cpp")
# 添加 YSolowork 库
add_library(YSolowork STATIC ${YSolowork_SOURCES})
# 链接 Boost::process 和 Boost::system
target_link_libraries(YSolowork PRIVATE Boost::process Boost::system)
# _WIN32_WINNT 符号的定义
if (MSVC)
    # Windows 平台需要定义 _WIN32_WINNT=0x0A00 表示 Windows 10, 支持的最低 Windows 版本
    # 这是 boost 需要的
    target_compile_definitions(YSolowork PRIVATE _WIN32_WINNT=0x0A00)
endif()

# 添加 OpenCL 库
find_package(OpenCL REQUIRED)
target_link_libraries(YSolowork PRIVATE OpenCL::OpenCL)

# 设置 YSolowork 库编译选项
target_compile_features(YSolowork PRIVATE cxx_std_20)
set_target_properties(YSolowork PROPERTIES
    CXX_STANDARD_REQUIRED YES
)

# 添加 Drogon 库
# find_package(Drogon REQUIRED) 在 Windows 上编译导致错误，改用下面的方式
# 添加 Drogon 子模块
# 不知为何 drogon 的 check_include_file_cxx(coroutine HAS_COROUTINE) 无法检测到 coroutine 头文件，所以这里强制开启 coroutine
# set(HAS_COROUTINE ON CACHE BOOL "Force enable coroutine support for all subprojects" FORCE)
add_subdirectory(
    ${YSolowork_SOURCE_DIR}/vendor/drogon
    ${YSolowork_BINARY_DIR}/vendor/drogon
)
cmake_policy(SET CMP0079 NEW) # 3.13 之后的版本需要设置这个来允许跨目录链接

# 添加选项来控制是否构建 YLineServer 和 YLineWorker
option(BUILD_YLINESERVER "Build YLineServer" ON)
option(BUILD_YLINEWORKER "Build YLineWorker" ON)
option(YSolowork_QUIET "Quiet build" OFF)

# 指定 NVML 路径
set(NVML_LIBRARY_INCLUDE ${YSolowork_SOURCE_DIR}/vendor_prebuild/nvml/inc)  # nvml 头文件路径
message("NVML_LIBRARY_INCLUDE: ${NVML_LIBRARY_INCLUDE}")
# 添加 NVML 头文件路径
# 只需要添加头文件，在运行时检测加载 nvml, 这将兼容非 NVIDIA 硬件的机器
# only add include directory, load nvml at runtime, this will be compatible with non-NVIDIA hardware
target_include_directories(YSolowork PRIVATE ${NVML_LIBRARY_INCLUDE})

# print build config message
include(cmake/YSolowork_message.cmake)

# 添加 YLineServer
if (BUILD_YLINESERVER)
    add_subdirectory(YLineServer)
endif()

# 添加 YLineWorker
if (BUILD_YLINEWORKER)
    add_subdirectory(YLineWorker)
endif()