@echo off
echo Running CMake with Ninja generator...
cmake -G Ninja -B target -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DUSE_SPDLOG=ON -DBUILD_SQLITE=OFF -DBUILD_EXAMPLES=OFF -DBUILD_YAML_CONFIG=OFF -D_WIN32_WINNT=0x0A00
