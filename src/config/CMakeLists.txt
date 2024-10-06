# 设置库名称
set(CONFIG_LIB_NAME config)

# 指定源文件和头文件
set(CONFIG_SOURCES
    ConfigLoader.cpp  # 加载配置文件的实现
)

set(CONFIG_HEADERS
    ConfigLoader.hpp  # 加载配置文件的头文件
    ConfigSubject.hpp  # 主题类头文件
    ConfigObserver.hpp  # 观察者类头文件
)

# 创建静态库
add_library(${CONFIG_LIB_NAME} STATIC ${CONFIG_SOURCES} ${CONFIG_HEADERS})

# 设置库的输出目录
set_target_properties(${CONFIG_LIB_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/output  # 静态库的输出目录
)

# 添加目标包含目录
target_include_directories(${CONFIG_LIB_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}  # 允许其他模块引用此库时使用的头文件目录
)

# 如果需要链接其他库，可以在这里添加
find_package(nlohmann_json REQUIRED)  # 查找 nlohmann_json 库
target_link_libraries(${CONFIG_LIB_NAME} PUBLIC nlohmann_json::nlohmann_json)
