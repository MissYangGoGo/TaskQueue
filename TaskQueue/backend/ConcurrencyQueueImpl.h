#ifndef __PARALLEL_TASK_QUEUE_IMPL_H__
#define __PARALLEL_TASK_QUEUE_IMPL_H__

#include "IQueueImpl.h"
namespace task
{
class ConcurrencyQueueImpl : public IQueueImpl
{
public:
    ConcurrencyQueueImpl(TaskQueuePriority prio, const ThreadPoolPtr& threadPool);
    ~ConcurrencyQueueImpl();

    virtual void async(const TaskOperatorPtr& task) override;
    virtual void sync(const TaskOperatorPtr& task, std::chrono::milliseconds timeout = std::chrono::milliseconds(-1)) override;
    virtual void after(std::chrono::milliseconds delay, const TaskOperatorPtr& task) override;

private:
    TaskQueuePriority mPriority{ TaskQueuePriority::TQP_Normal };
};

}  // namespace task

#endif  // __PARALLEL_TASK_QUEUE_IMPL_H__