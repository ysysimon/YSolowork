# YSolowork
YSolowork 我对一系列用着不舒服的 CG 生产工具重新造的轮子，包含
1. **YLineServer**: 分布式任务执行系统的服务端
2. **YLineWorker**: 分布式任务执行系统的执行端 
3. **CGSolowork** (计划中): 以 **OpenUSD** 为核心的协同工作系统 

### 开发依赖

#### YSolowork
- Boost.process (建议使用官方repo安装)
    - Boost.system
- OpenCL (for device info)

#### YLineServer
- spdlog
- botan (建议使用官方repo安装)
- Drogon (build with project)
    - jsoncpp
    - uuid
    - zlib
    - libpq (PostgreSQL)
- toml++ (header only) (build with project)
- jwt++ (build with project)

#### YLineWorker
- magic_enum
- spdlog
- Drogon (build with project)
    - jsoncpp
    - uuid
    - zlib
    - libpq (PostgreSQL)
- toml++ (header only) (build with project)
