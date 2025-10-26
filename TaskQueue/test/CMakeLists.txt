# 测试目录的CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(dispatch_queue_tests)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加编译选项
add_compile_options(-Wall -Wextra -g)

# 包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../backend)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../common)

# 设置主项目构建目录
set(MAIN_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(MAIN_PROJECT_BUILD_DIR ${MAIN_PROJECT_DIR}/build)

# 查找所有测试源文件
file(GLOB TEST_SOURCES "*.cpp")

# 创建测试可执行文件
foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    
    # 跳过主测试文件（如果有的话）
    if(NOT ${TEST_NAME} STREQUAL "Test")
        add_executable(${TEST_NAME} ${TEST_SOURCE})
        
        # 链接必要的库
        target_link_libraries(${TEST_NAME} 
            ${MAIN_PROJECT_BUILD_DIR}/lib/libdispatch_queue.dylib
            pthread
        )
        
        # 设置输出目录
        set_target_properties(${TEST_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin
        )
    endif()
endforeach()

# 创建一个运行所有测试的目标
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run_all_tests.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin
    COMMENT "Running all tests"
)

message(STATUS "Test executables will be built in: ${CMAKE_CURRENT_BINARY_DIR}/bin")