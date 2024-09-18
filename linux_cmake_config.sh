#!/bin/bash
echo "Running CMake with Ninja generator..."
cmake -G Ninja -B target -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# if std not found on Linux, maybe need to install libstdc++-12-dev
# use .clangd instead of copy compile_commands.json,
# # 检查 CMake 是否成功运行
# if [ $? -eq 0 ]; then
#     echo "CMake generation successful."
    
#     # 检查 compile_commands.json 是否存在
#     if [ -f target/compile_commands.json ]; then
#         echo "Found compile_commands.json. Copying to project root..."
        
#         # 复制 compile_commands.json 到项目根目录
#         cp target/compile_commands.json .
        
#         echo "compile_commands.json copied to project root."
#     else
#         echo "Error: compile_commands.json not found in target directory."
#         exit 1
#     fi
# else
#     echo "Error: CMake generation failed."
#     exit 1
# fi
