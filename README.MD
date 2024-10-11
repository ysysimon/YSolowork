# YSolowork
YSolowork 我对一系列用着不舒服的 CG 生产工具重新造的轮子，包含
1. **YLineServer**: 分布式任务执行系统的服务端
2. **YLineWorker**: 分布式任务执行系统的执行端 
3. **CGSolowork** (计划中): 以 **OpenUSD** 为核心的协同工作系统 

### 开发依赖 dev depend

#### YSolowork
- Boost.process (plz using the official repo to install 建议使用官方repo安装)
    - Boost.system
- OpenCL (for device info)

#### YLineServer
- spdlog
- botan (plz using the official repo to install 建议使用官方repo安装)
- Drogon (build with project)
    - jsoncpp
    - uuid
    - zlib
    - libpq (PostgreSQL)
- toml++ (header only) (build with project)
- jwt++ (build with project)

#### YLineWorker
- spdlog
- magic_enum (header only) (build with project)
- Drogon (build with project)
    - jsoncpp
    - uuid
    - zlib
- toml++ (header only) (build with project)

### runtime depend

#### Windows
- intel-runtime-opencl (for x86 CPU (Intel/AMD) device detect 用于 x86 CPU (Intel/AMD) 设备检测)

#### Linux
- intel-oneapi-runtime-opencl (for x86 CPU (Intel/AMD) device detect 用于 x86 CPU (Intel/AMD) 设备检测)
- intel-oneapi-runtime-libs