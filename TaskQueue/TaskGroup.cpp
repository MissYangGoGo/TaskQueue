#include "TaskGroup.h"
#include "TaskQueueDefine.h"
#include "backend/GroupImpl.h"
#include "TaskQueueFactory.h"
#include "TaskQueue.h"
namespace task
{
void TaskGroup::asyncQueue(const TaskOperatorPtr& task, const TaskQueuePtr& queue)
{
    auto q = queue;
    if (q == nullptr)
    {
        q = TaskQueueFactory::GetInstance().globalConcurrencyQueue(TaskQueuePriority::TQP_Normal);
    }
    mGroupImpl->async(task, q);
}

void TaskGroup::async(const TaskOperatorPtr& task, TaskQueuePriority priority)
{
    mGroupImpl->async(task, priority);
}

void TaskGroup::notify(const TaskOperatorPtr& task, const TaskQueuePtr& queue)
{
    auto q = queue;
    if (q == nullptr)
    {
        q = TaskQueueFactory::GetInstance().globalConcurrencyQueue(TaskQueuePriority::TQP_Normal);
    }
    mGroupImpl->notify(task, q);
}

bool TaskGroup::wait(std::chrono::milliseconds t)
{
    return mGroupImpl->wait(t);
}

}  // namespace task