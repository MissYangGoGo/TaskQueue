#ifndef TASK_QUEUE_CONSTANT_H
#define TASK_QUEUE_CONSTANT_H

#include <cstdint>
#include <chrono>
namespace task
{

class TaskQueueConstant
{
public:
    static uint32_t sTaskDurationThreshold;  // 单个任务耗时阈值

    static uint32_t             sMaxTaskQueueCount;        // 最大任务队列数量
    static uint32_t             sMaxSpinCount;             // 自旋尝试获取信号量的次数
    static uint32_t             sMaxReportCountThreshold;  // 上报最大次数阈值
    static std::chrono::seconds sMaxSleepTimeout;          // 线程调度，最大等待时间2分钟还没有任务，可以自动退出 [独占线程除外]

    static uint32_t sOneMinuteMillisCount;   // 一分钟的毫秒数
    static uint32_t sBlockTimeoutThreshold;  // 卡顿检查阈值

    // 更新一次配置
    static void updateConfig();
};

}  // namespace task

#endif  //TASK_QUEUE_CONSTANT_H