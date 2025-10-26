#include "SerialQueueImpl.h"

#include "LWBarrier.h"
#include "TaskOperatorBackend.h"
#include "common/LogHelper.h"
#include "TaskQueueDefine.h"

#include <cassert>
#include <cstdio>
#include <memory>
namespace task
{
SerialQueueImpl::SerialQueueImpl(const std::string&   label,
                                 bool                 isExclusive,
                                 const ThreadPoolPtr& threadPool,
                                 WorkThreadPriority   prio)
    : IQueueImpl(TaskQueueType::TQT_Serial, threadPool)
    , mIsExclusive(isExclusive)
{
    LOGE("[TASK]SerialQueueImpl::SerialQueueImpl, label: %s, isExclusive: %d, prio: %d", label.c_str(), isExclusive, prio);
    if (mIsExclusive)
    {
        mThreadId = _threadPool()->attachOneThread(label, prio);
    }
    else
    {
        // 非独占模式下，创建一个串行任务
        mSerialTask = std::make_shared<TaskOperator>([this](const TaskOperatorPtr& /*task*/) {
            _processTask();
        });
    }
}

SerialQueueImpl::~SerialQueueImpl()
{
    LOGE("[TASK]SerialQueueImpl::~SerialQueueImpl, mIsExclusive: %d, mThreadId: %d\n", mIsExclusive, mThreadId);
    if (mIsExclusive && mThreadId >= 0)
    {
        _threadPool()->detachOneThread(mThreadId);
    }
}

void SerialQueueImpl::_processTask()
{
    // 1. 从队列中获取一个任务进行执行
    TaskOperatorPtr op;
    if (mTasks.try_dequeue(op) && op)
    {
        (*op)();
    }

    // 2. 处理队列中的下一个任务
    if (mTasks.size_approx() > 0)
    {
        _threadPool()->execute(mSerialTask);
    }
    else
    {
        // 3.队列没有任务，重置同步标志位
        mSyncFlag.clear();
        //重置后再检查一次， 确保无任务抛入
        if (mTasks.size_approx() > 0 && !mSyncFlag.test_and_set())
        {
            _threadPool()->execute(mSerialTask);
        }
    }
}

void SerialQueueImpl::async(const TaskOperatorPtr& task)
{
    assert(_threadPool());
    task->resetCallStartTime();

    if (mIsExclusive)
    {
        _threadPool()->execute(task, mThreadId);
    }
    else
    {
        mTasks.enqueue(task);
        // 如果mSyncFlag为false, 则设置为true, 并执行任务,
        // 注意：返回值为prev值
        if (!mSyncFlag.test_and_set(std::memory_order_acq_rel))
        {
            _threadPool()->execute(mSerialTask);
        }
    }
}

void SerialQueueImpl::sync(const TaskOperatorPtr& task, std::chrono::milliseconds timeout)
{
    assert(_threadPool());
    // 转为一个 BarrierTaskOperator
    task->resetCallStartTime();

    auto syncTask = std::make_shared<TaskBarrierOperator>(task);
    if (mIsExclusive)
    {
        _threadPool()->execute(syncTask, mThreadId);
    }
    else
    {
        mTasks.enqueue(syncTask);
        if (!mSyncFlag.test_and_set(std::memory_order_acq_rel))
        {
            _threadPool()->execute(mSerialTask);
        }
    }
    syncTask->wait(timeout);
}

void SerialQueueImpl::after(std::chrono::milliseconds delay, const TaskOperatorPtr& task)
{
    assert(_threadPool());
    task->resetCallStartTime();

    auto delayTask = std::make_shared<TaskDelayOperator>(delay, task);
    if (mIsExclusive)
    {
        _threadPool()->execute(delayTask, mThreadId);
    }
    else
    {
        mTasks.enqueue(delayTask);
        if (!mSyncFlag.test_and_set(std::memory_order_acq_rel))
        {
            _threadPool()->execute(mSerialTask);
        }
    }
}
}  // namespace task