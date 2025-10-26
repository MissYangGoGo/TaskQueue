#include "ConcurrencyQueueImpl.h"
#include "TaskOperatorBackend.h"
#include <cassert>
#include <cstdio>
#include "common/LogHelper.h"
namespace task
{

ConcurrencyQueueImpl::ConcurrencyQueueImpl(TaskQueuePriority prio, const ThreadPoolPtr& threadPool)
    : IQueueImpl(TaskQueueType::TQT_Parallel, threadPool)
    , mPriority(prio)
{
    LOGE("[TASK]ConcurrencyQueueImpl() %d\n", prio);
}

ConcurrencyQueueImpl::~ConcurrencyQueueImpl()
{
    LOGE("[TASK]~ConcurrencyQueueImpl() %d\n", mPriority);
}

void ConcurrencyQueueImpl::async(const TaskOperatorPtr& task)
{
    assert(_threadPool());
    task->resetCallStartTime();

    _threadPool()->execute(task, mPriority);
}

void ConcurrencyQueueImpl::sync(const TaskOperatorPtr& task, std::chrono::milliseconds timeout)
{
    assert(_threadPool());
    task->resetCallStartTime();

    auto syncTask = std::make_shared<TaskBarrierOperator>(task);
    _threadPool()->execute(syncTask, mPriority);
    syncTask->wait(timeout);
}

void ConcurrencyQueueImpl::after(std::chrono::milliseconds delay, const TaskOperatorPtr& task)
{
    assert(_threadPool());
    task->resetCallStartTime();
    auto delayTask = std::make_shared<TaskDelayOperator>(delay, task);
    _threadPool()->execute(delayTask, mPriority);
}

}  // namespace task