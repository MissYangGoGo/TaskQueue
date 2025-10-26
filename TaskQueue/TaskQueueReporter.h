#ifndef TASK_QUEUE_REPORTER_H
#define TASK_QUEUE_REPORTER_H

// 上报数据
// 1. 线程数量(active， idle， max)
// 2. 线程名称， 线程对应任务数量，以及堆栈信息
// 3. 收集任务类型， 任务执行时间统计

// 上报时机
// 1. 线程数量变化超过阈值时上报
// 2. 单个任务耗时超过阈值时上报
// 3. 任务数量超出阈值时上报
// 4. 间隔性上报统计信息

#include "common/HESingleton.h"
#include <string>

namespace task
{

enum class TaskQueueReporterType
{
    TQRT_None                        = 0,
    TQRT_ThreadCountChanged          = 1,  // 线程数量变化
    TQRT_TaskDurationExceedThreshold = 2,  // 单个任务耗时超过阈值
    TQRT_TaskCountExceedThreshold    = 3,  // 任务数量超出阈值
};

class TaskQueueReporter : public task::HESingleton<TaskQueueReporter>
{
    friend class task::HESingleton<TaskQueueReporter>;

public:
    // 根据类型通知上报
    void notifyReport(TaskQueueReporterType type, std::string&& msg = "");

    // 最多上报多少次
    void notifyReportLimit(int32_t limit, int32_t& val, TaskQueueReporterType type, std::string&& msg = "");

private:
    void _reportThreadCountChanged(std::string&& msg);
    void _reportTaskDurationExceedThreshold(std::string&& msg);
    void _reportTaskCountExceedThreshold(std::string&& msg);
};
}  // namespace task

#endif