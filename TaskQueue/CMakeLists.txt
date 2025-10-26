cmake_minimum_required(VERSION 3.14)

# 设置项目名称和版本
project(dispatch_queue VERSION 1.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -g -O0")

# 生成编译数据库，用于IDE代码导航和补全
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 收集所有源文件
file(GLOB DISPATCH_QUEUE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/backend/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp"
)

file(GLOB DISPATCH_QUEUE_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/backend/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/common/*.h"
)

# 判断平台
if(ANDROID)
    # Android平台配置
    add_library(${PROJECT_NAME} SHARED ${DISPATCH_QUEUE_SOURCES} ${DISPATCH_QUEUE_HEADERS})
    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_ANDROID)
    
    # 链接Android所需的库
    target_link_libraries(${PROJECT_NAME} PRIVATE log)

elseif(IOS)
    # iOS平台配置
    add_library(${PROJECT_NAME} STATIC ${DISPATCH_QUEUE_SOURCES} ${DISPATCH_QUEUE_HEADERS})
    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_IOS)
    
    # 设置iOS特定的编译选项
    set_target_properties(${PROJECT_NAME} PROPERTIES
        XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "9.0"
        XCODE_ATTRIBUTE_ENABLE_BITCODE "NO"
    )

else()
    # macOS平台配置（默认）
    add_library(${PROJECT_NAME} SHARED ${DISPATCH_QUEUE_SOURCES} ${DISPATCH_QUEUE_HEADERS})
    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_MACOS)
    
    # 设置macOS特定的编译选项
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_RPATH ON
    )
endif()

# 添加头文件搜索路径
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/backend
    ${CMAKE_CURRENT_SOURCE_DIR}/common
)

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)