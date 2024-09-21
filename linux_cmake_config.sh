#!/bin/bash
echo "Running CMake with Ninja generator..."
cmake -G Ninja -B target -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DUSE_SPDLOG=ON -DBUILD_SQLITE=OFF

# if std not found on Linux, maybe need to install libstdc++-12-dev
# use .clangd instead of copy compile_commands.json
