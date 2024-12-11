# YSolowork
YSolowork 我对一系列用着不舒服的 CG 生产工具重新造的轮子，包含
1. **YLineServer**: 分布式任务执行系统的服务端
2. **YLineWorker**: 分布式任务执行系统的执行端 
3. **CGSolowork** (计划中): 以 **OpenUSD** 为核心的协同工作系统 

### 开发依赖 dev depend
gcc-13 (经过测试)
MSVC 19.4 (经过测试)
如果使用 clangd 作为语法提示，请使用最新版以避免 C++20 错误提示

#### YSolowork
- Boost (plz using the official repo to install 建议使用官方repo安装)
    - process
    - system
    - uuid
    - interprocess
    - graph
- OpenCL (for device info)

#### YLineServer
- spdlog
- botan (plz using the official repo to install 建议使用官方repo安装)
- Drogon (build with project)
    - jsoncpp
    - uuid (no need for windows)
    - zlib
    - libpq (PostgreSQL)
    - hiredis (redis) (build with project)
- toml++ (header only) (build with project)
- jwt++ (build with project)
- entt (build with project)
- tl::expected (header only) (build with project)

#### YLineWorker
- spdlog
- magic_enum (header only) (build with project)
- Drogon (build with project)
    - jsoncpp
    - uuid (no need for windows)
    - zlib
- toml++ (header only) (build with project)

### runtime depend
- dbmate (use to migrate database 用于数据库迁移)
    - (will download automatically, if have download problem, download here)
      (应当自动下载，如有下载问题，请在此处下载)
        - https://github.com/amacneil/dbmate 

#### Windows
- intel-runtime-opencl (for x86 CPU (Intel/AMD) device detect 用于 x86 CPU (Intel/AMD) 设备检测)

#### Linux
- intel-oneapi-runtime-opencl (for x86 CPU (Intel/AMD) device detect 用于 x86 CPU (Intel/AMD) 设备检测)
- intel-oneapi-runtime-libs (for libsvml.so)