#pragma once

#include <cstdio>
#include <iostream>
#include <chrono>
#include <iomanip>

namespace task 
{

// 递归终止
inline void printImpl() {}

// 递归展开参数
template<typename T, typename... Args>
void printImpl(T&& first, Args&&... rest) {
    std::cout << std::forward<T>(first);
    if constexpr (sizeof...(rest) > 0) {
        std::cout << " ";
        printImpl(std::forward<Args>(rest)...);
    }
}

// 主函数
template<typename... Args>
void print(Args&&... args) {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    auto timer = std::chrono::system_clock::to_time_t(now);
    std::tm bt = *std::localtime(&timer);
    
    std::cout << "[" << std::put_time(&bt, "%H:%M:%S") << "." 
              << std::setfill('0') << std::setw(3) << ms.count() << "] ";
    printImpl(std::forward<Args>(args)...);
    std::cout << std::endl;
}

// printf 风格的格式化输出
template<typename... Args>
void printf(const char* format, Args&&... args) {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    auto timer = std::chrono::system_clock::to_time_t(now);
    std::tm bt = *std::localtime(&timer);
    
    char time_buffer[32];
    std::snprintf(time_buffer, sizeof(time_buffer), "[%02d:%02d:%02d.%03d] ",
                  bt.tm_hour, bt.tm_min, bt.tm_sec, static_cast<int>(ms.count()));
    std::printf("%s", time_buffer);
    if constexpr (sizeof...(args) > 0) {
        std::printf(format, std::forward<Args>(args)...);
    } else {
        std::printf("%s", format);
    }
    std::printf("\n");
}

} // namespace log


#define LOGD(format, args...) task::printf(format, ##args)
#define LOGI(format, args...) task::printf(format, ##args)
#define LOGW(format, args...) task::printf(format, ##args)
#define LOGE(format, args...) task::printf(format, ##args)

