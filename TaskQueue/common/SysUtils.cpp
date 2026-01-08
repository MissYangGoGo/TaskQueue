
#include "SysUtils.h"

#include <thread>
namespace task
{

void SysUtils::cpuYield()
{
// defined(__arm__)：所有32位ARM（包括ARMv6、ARMv7等）
// defined(_ARM_ARCH_7)：只在ARMv7及以上有效
// defined(__thumb__)：只在Thumb模式下有效
// defined(__arm64__)：64位ARM
// 以上平台才支持yield指令
// #if (defined(__arm__) && defined(_ARM_ARCH_7) && defined(__thumb__)) || defined(__arm64__)
//     __asm__("yield");
// #else
//     __asm__("");
// #endif

    std::this_thread::yield();
}

uint16_t SysUtils::cpuCount()
{
    return std::thread::hardware_concurrency();
}

}  // namespace task