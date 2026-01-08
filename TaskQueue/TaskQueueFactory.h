// 队列工厂， 用于创建或获取对应队列
#ifndef __TASK_QUEUE_FACTORY_H__
#define __TASK_QUEUE_FACTORY_H__

#include "common/HESingleton.h"
#include "TaskQueueDefine.h"
#include <memory>
#include <string>

namespace task
{
// 队列工厂
class TaskQueueFactoryImpl;
class TaskQueueFactory : public task::HESingleton<TaskQueueFactory>
{
    friend class task::HESingleton<TaskQueueFactory>;

public:
    // 创建串行队列
    // label: 队列名称
    // priority: 队列优先级
    // isExclusive: 是否独占线程
    TaskQueuePtr createSerialTaskQueue(const std::string& label, WorkThreadPriority priority, bool isExclusive);

    // 创建并行队列
    // label: 队列名称
    // priority: 队列优先级
    TaskQueuePtr createConcurrencyTaskQueue(const std::string& label, TaskQueuePriority priority);

    // 获取队列 【全局并行队列】
    // priority: 队列优先级
    TaskQueuePtr& globalConcurrencyQueue(TaskQueuePriority priority);

    // 获取串行队列 【全局串行队列， 非独占线程】
    TaskQueuePtr& globalSerialQueue();

    // 创建group
    TaskGroupPtr createTaskGroup();

private:
    const std::shared_ptr<TaskQueueFactoryImpl>& _getFactoryImpl();
    std::shared_ptr<TaskQueueFactoryImpl>        mFactoryImpl;
};

}  // namespace task

#endif  // __TASK_QUEUE_FACTORY_H__