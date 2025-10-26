// 任务组
// 任务组是一组任务的集合，任务组的任务会被分配到不同的线程中执行

#ifndef __TASK_GROUP_H__
#define __TASK_GROUP_H__

#include <chrono>
#include <cstddef>
#include <memory>
#include "TaskOperator.h"
#include "TaskQueueDefine.h"
namespace task
{
class GroupImpl;
class TaskGroup final
{
public:
    using Func = std::function<void()>;

public:
    explicit TaskGroup(const std::shared_ptr<GroupImpl>& groupImpl)
        : mGroupImpl(groupImpl)
    {
    }

    ~TaskGroup() = default;

    // 在指定队列抛一个任务
    void asyncQueue(const TaskOperatorPtr& task, const TaskQueuePtr& queue = nullptr);
    void asyncQueue( Func&& f, const TaskQueuePtr& queue = nullptr)
    {
        auto task = std::make_shared<TaskOperator>([f](const TaskOperatorPtr&) {
            f();
        });
        asyncQueue(task, queue);
    }

    // 在全局队列抛一个任务
    void async(const TaskOperatorPtr& task, TaskQueuePriority priority = TaskQueuePriority::TQP_Normal);
    void async( Func&& f, TaskQueuePriority priority = TaskQueuePriority::TQP_Normal)
    {
        auto task = std::make_shared<TaskOperator>([f](const TaskOperatorPtr&) {
            f();
        });
        async(task, priority);
    }

    // group所有执行完后，在指定queue上异步通知
    void notify(const TaskOperatorPtr& task, const TaskQueuePtr& queue = nullptr);
    void notify( Func&& f, const TaskQueuePtr& queue = nullptr)
    {
        auto task = std::make_shared<TaskOperator>([f](const TaskOperatorPtr&) {
            f();
        });
        notify(task, queue);
    }

    // group 等待所有任务结束
    bool wait(std::chrono::milliseconds t = std::chrono::milliseconds(-1));

private:
    std::shared_ptr<GroupImpl> mGroupImpl;
};

}  // namespace task

#endif  // __TASK_GROUP_H__