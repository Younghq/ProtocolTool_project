# 设置cmake版本
cmake_minimum_required(VERSION 3.10)

# 设置项目名称与编译的C++版本
project(ProtocolTool_project)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 指定构建输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/output) # 静态库的输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/output) # 动态库的输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/output) # 可执行文件的输出目录


# 查找主程序源文件
file(GLOB_RECURSE MAIN_SOURCES "src/main.cpp") 

# 包含子模块
# add_subdirectory(src/network)  # 包含network模块的CMakeLists.txt
# add_subdirectory(src/protocol)  # 包含protocol模块的CMakeLists.txt

# 生成可执行文件
add_executable(ProtocolTool ${MAIN_SOURCES})

# 链接子模块生成的库
# target_link_libraries(MyExecutable handleNet protocolLib)  
