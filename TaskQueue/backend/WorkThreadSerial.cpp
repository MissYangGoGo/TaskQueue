#include "WorkThreadSerial.h"
#include "common/LogHelper.h"
#include "TaskQueueConstant.h"
#include "TaskOperator.h"
#include "TaskQueueConstant.h"
#include "common/HETimerHelper.h"
#include "TaskQueueReporter.h"
#include "common/LogHelper.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <functional>
#include <mutex>

namespace task
{

WorkThreadSerial::WorkThreadSerial(const std::weak_ptr<IThreadPool>& threadPool)
    : WorkThreadBase(threadPool)
{
    mThread = std::thread(&WorkThreadSerial::_run, this);
    LOGE("[TASK]WorkThreadSerial::WorkThreadSerial, threadId: %d", threadId());
}

WorkThreadSerial::~WorkThreadSerial()
{
    LOGE("[TASK]WorkThreadSerial::~WorkThreadSerial, join before threadId: %d", threadId());
    cancel();
    if (mThread.joinable())
    {
        mThread.join();
    }
    LOGE("[TASK]WorkThreadSerial::~WorkThreadSerial, join after threadId: %d", threadId());
}

void WorkThreadSerial::_run()
{
    // comm::HEThreadScopeMonitor monitor(getName(), comm::HEThreadMonitor::GetInstance().getCurrentThreadId());
    LOGE("[TASK]WorkThreadSerial::run, threadId: %d, name: %s", threadId(), mName.c_str());
    while (!mIsCancelled.load(std::memory_order_acquire))
    {
        _changeName();
        _changePriority();

        // 独占线程不自动退出, 需用户主动取消(调用cancel方法)
        _exclusive();
    }

    // 线程结束，从线程池中移除, 结束自己
    auto pool = _getThreadPool();
    if (pool)
    {
        pool->unregisterWorkThread(shared_from_this());
    }

    LOGE("[TASK]WorkThreadSerial::run, threadId: %d, exit, name: %s\n", threadId(), mName.c_str());
}

bool WorkThreadSerial::commit(const TaskOperatorPtr& task)
{
    bool ret = mWorkQueue.enqueue(task);
    mSemaphore.release();
    _monitorTask();
    return ret;
}

bool WorkThreadSerial::commit(TaskOperatorPtr&& task) noexcept
{
    bool ret = mWorkQueue.enqueue(std::move(task));
    mSemaphore.release();
    _monitorTask();
    return ret;
}

void WorkThreadSerial::_exclusive()
{
    // 等待信号量
    mSemaphore.waitAcquire(TaskQueueConstant::sMaxSleepTimeout);

    // 执行任务
    TaskOperatorPtr op;
    if (mWorkQueue.try_dequeue(op) && op)
    {
        mCurrTask     = op;
        mStartRunTime = task::HETimerHelper::currentTimeMillis();
        mIsRunning    = true;
        if(!op->isCancelled())
        {
            (*op)();
        }
        mIsRunning = false;

        //收集统计信息
        _updateStat(op);
    }
}

void WorkThreadSerial::setActive(bool active)
{
    mIsActive = active;
    if (!active)
    {
        mCurrTask = nullptr;
    }
}

bool WorkThreadSerial::isBlocked() const
{
    // 认为isRunning = true  且 running时长大于5秒，为线程卡住
    return mIsRunning && (task::HETimerHelper::currentTimeMillis() - mStartRunTime > TaskQueueConstant::sBlockTimeoutThreshold);
}

void WorkThreadSerial::_monitorTask()
{
    //1. 监控任务队列数量
    if (mWorkQueue.size_approx() >= TaskQueueConstant::sMaxTaskQueueCount)
    {
        TaskQueueReporter::GetInstance().notifyReportLimit(TaskQueueConstant::sMaxReportCountThreshold,
                                                           mReportCount,
                                                           TaskQueueReporterType::TQRT_TaskCountExceedThreshold,
                                                           blockedInfo());
    }
    else
    {
        mReportCount = 0;
    }
}

std::string WorkThreadSerial::blockedInfo()
{
    auto msg = _statInfo()
               + " tskcnt:"
               + std::to_string(mWorkQueue.size_approx())
               + " blkTime:"
               + std::to_string(task::HETimerHelper::currentTimeMillis() - mStartRunTime);

    LOGE("[TASK] ", msg.c_str());
    return msg;
}

}  // namespace task