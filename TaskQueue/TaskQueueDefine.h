// 对外定义枚举
#ifndef __TASK_QUEUE_DEFINE_H__
#define __TASK_QUEUE_DEFINE_H__

#include <cstdint>
#include <memory>

namespace task
{

// 队列类型
enum class TaskQueueType : std::uint8_t
{
    TQT_Serial = 0,  // 串行队列， 任务按顺序执行 (可能会在不同线程)
    TQT_Parallel,    // 并行队列， 任务并行执行
    TQT_Count,
};

// 队列优先级 【只是说明任务执行的先后顺序，不涉及线程优先级】
// 此用于并行队列
enum class TaskQueuePriority : std::uint8_t
{
    TQP_Low = 0,
    TQP_Normal,
    TQP_High,
    TQP_Count,
};

// 工作线程优先级【只用于独占任务队列】
// 此用于串行独占队列
enum class WorkThreadPriority : std::uint8_t
{
    WTP_Low = 0,
    WTP_Normal,
    WTP_High,
    WTP_Count,
};

class TaskQueue;
class TaskGroup;
class IQueueImpl;
class TaskOperator;
}  // namespace task

// alias
using TaskQueuePtr     = std::shared_ptr<task::TaskQueue>;
using TaskGroupPtr     = std::shared_ptr<task::TaskGroup>;
using TaskQueueImplPtr = std::shared_ptr<task::IQueueImpl>;
using TaskOperatorPtr  = std::shared_ptr<task::TaskOperator>;
#endif