# 设置cmake版本
cmake_minimum_required(VERSION 3.10)

# 设置项目名称与编译的C++版本
project(ProtocolTool_project)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)  # 是否要断点调试，不要就注释

# 指定构建输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/output) # 静态库的输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/output) # 动态库的输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/output) # 可执行文件的输出目录

file(GLOB_RECURSE MAIN_SOURCES "src/main.cpp") 

# 包含子模块
add_subdirectory(src/config)  # 包含network模块的CMakeLists.txt

# 生成可执行文件
add_executable(ProtocolTool ${MAIN_SOURCES})

# 链接子模块生成的库
target_link_libraries(ProtocolTool config)  

# 在构建后移动 ./public/* 到输出目录
set(PUBLIC_FILES "${CMAKE_SOURCE_DIR}/public/*")
set(OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

add_custom_command(
    TARGET ProtocolTool  # 依赖于 ProtocolTool 可执行文件
    POST_BUILD  # 在目标构建后执行
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PUBLIC_FILES} ${OUTPUT_DIR}
)