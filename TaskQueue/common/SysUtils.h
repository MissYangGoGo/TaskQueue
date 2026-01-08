#ifndef __SYSUTILS_H__
#define __SYSUTILS_H__

#include <stdint.h>
namespace task
{

class SysUtils
{
public:
    static void     cpuYield();
    static uint16_t cpuCount();
};
}  // namespace task

#endif  // __SYSUTILS_H__
