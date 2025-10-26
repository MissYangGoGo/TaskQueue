#include "TaskQueueConstant.h"
#include <cstdint>
#include <chrono>
namespace task
{
uint32_t TaskQueueConstant::sTaskDurationThreshold = 2000;  //2s阈值

uint32_t             TaskQueueConstant::sMaxTaskQueueCount       = 10;                         // 最大任务队列数量
uint32_t             TaskQueueConstant::sMaxReportCountThreshold = 5;                          // 上报最大次数阈值
std::chrono::seconds TaskQueueConstant::sMaxSleepTimeout         = std::chrono::seconds(120);  // 线程调度，最大等待时间2分钟还没有任务，可以自动退出 [独占线程除外]
uint32_t             TaskQueueConstant::sBlockTimeoutThreshold = 5000;

// 固定参数
uint32_t             TaskQueueConstant::sOneMinuteMillisCount  = 60000;
uint32_t             TaskQueueConstant::sMaxSpinCount            = 10;                         // 自旋尝试获取信号量的次数

void TaskQueueConstant::updateConfig()
{
    // todo
}

}  // namespace task