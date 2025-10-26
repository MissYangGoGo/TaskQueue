#include "WorkThreadConcurrency.h"
#include "common/LogHelper.h"
#include "TaskQueueConstant.h"
#include "TaskOperator.h"
#include "common/HETimerHelper.h"
#include "common/LogHelper.h"
#include <cassert>
#include <chrono>
#include <cstdio>
namespace task
{
WorkThreadConcurrency::WorkThreadConcurrency(const std::weak_ptr<IThreadPool>& threadPool)
    : WorkThreadBase(threadPool)
{
    mThread = std::thread(&WorkThreadConcurrency::_run, this);
    mName   = "parallel_" + std::to_string(threadId());
    LOGE("[TASK] WorkThreadConcurrency::WorkThreadConcurrency, threadId: %d", threadId());
}

WorkThreadConcurrency::~WorkThreadConcurrency()
{
    LOGE("[TASK]WorkThreadConcurrency::~WorkThreadConcurrency, join before threadId: %d", threadId());
    cancel();
    if (mThread.joinable())
    {
        mThread.join();
    }
    LOGE("[TASK]WorkThreadConcurrency::~WorkThreadConcurrency, join after threadId: %d", threadId());
}

void WorkThreadConcurrency::_run()
{
    LOGE("[TASK]WorkThreadConcurrency::run, threadId: %d, name: %s", threadId(), mName.c_str());
    mNativeThreadId = std::this_thread::get_id();
    while (!mIsCancelled.load(std::memory_order_acquire))
    {
        _changeName();
        _changePriority();
        // 如果线程长时间没有任务执行，自动退出线程
        if (!_parallel())
        {
            break;
        }
    }

    // 线程结束，从线程池中移除, 结束自己
    auto pool = _getThreadPool();
    if (pool)
    {
        pool->unregisterWorkThread(shared_from_this());
    }

    LOGE("[TASK]WorkThreadConcurrency::run, threadId: %d, exit, name: %s\n", threadId(), mName.c_str());
}

bool WorkThreadConcurrency::_parallel()
{
    auto& data = _getThreadPool()->getData();
    assert(data);
    if (data == nullptr)
    {
        LOGE("[TASK][HY] WorkThreadConcurrency::_parallel, threadId: %d, threadPool is null", threadId());
        return false;
    }

    // 线程处于非执行状态，线程池空闲线程+1(所有线程同步看见 memory_order_seq_cst)
    data->mIdleThreads.fetch_add(1, std::memory_order_seq_cst);
    LOGE("[Job] WorkThreadConcurrency::_parallel 11, threadId: %d, idle: %d, threadPool is ready", threadId(), data->mIdleThreads.load());

    // 整个线程池数据，第一次尝试获取信号量【无锁】
    bool flag = data->mSemaphore.tryAcquire();
    if (!flag)
    {
        // 自旋尝试10次
        flag = data->mSemaphore.spinAcquire(TaskQueueConstant::sMaxSpinCount);
    }
    if (!flag)
    {
        // 等待信号量 或 超时
        flag = data->mSemaphore.waitAcquire(TaskQueueConstant::sMaxSleepTimeout);
        if (!flag)
        {
            // 超时了，尝试再获取一次信号量
            if (!data->mSemaphore.tryAcquire())
            {
                // 失败了，表示线程需要退出了
                return false;
            }
        }
    }

    // 处理同步死锁问题， 获取当前抛任务线程，是否为自身
    if(data->mInputThreadID == mNativeThreadId)
    {
        LOGE("[Job] WorkThreadConcurrency::_parallel, threadId: %d, threadPool is deadlock", threadId());
        LOGE("[Job] WorkThreadConcurrency::_parallel 22, threadId: %d, idle: %d, threadPool is ready", threadId(), data->mIdleThreads.load());

        data->mIdleThreads.fetch_sub(1, std::memory_order_seq_cst);

        // 通知线程池，当前线程不能执行任务
        auto pool = _getThreadPool();
        if (pool)
        {
            pool->notifyNextThread(threadId());
        }

        // 休眠10ms
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return true;
    }


    // 从高到低优先级 进行任务执行
    TaskOperatorPtr op;
    for (int i = ( int )TaskQueuePriority::TQP_High; i >= 0; --i)
    {
        if (data->mTaskQueues[i].try_dequeue(op) && op)
        {
            break;
        }
    }

    // 线程结束等待，线程池空闲线程-1
    data->mIdleThreads.fetch_sub(1, std::memory_order_seq_cst);
    LOGE("[Job] WorkThreadConcurrency::_parallel 33, threadId: %d, idle: %d, threadPool is ready", threadId(), data->mIdleThreads.load());
    if (op)
    {
        LOGE("[Job] WorkThreadConcurrency::_parallel getJob, threadId: %d, task: %p", threadId(), op.get());
        mCurrTask     = op;
        mStartRunTime = task::HETimerHelper::currentTimeMillis();
        mIsRunning    = true;
        if(!op->isCancelled()){
            (*op)();
        }
        mIsRunning = false;

        //收集统计信息
        _updateStat(op);
    }

    return true;
}

bool WorkThreadConcurrency::isBlocked() const
{
    // 运行超过5秒，为线程卡住
    return mIsRunning && (task::HETimerHelper::currentTimeMillis() - mStartRunTime > TaskQueueConstant::sBlockTimeoutThreshold);
}

std::string WorkThreadConcurrency::blockedInfo()
{
    auto msg = _statInfo() + " tid: " + std::to_string(threadId())
               + " blockT:" + std::to_string(task::HETimerHelper::currentTimeMillis() - mStartRunTime);

    LOGE("[TASK] %s \n", msg.c_str());
    return msg;
}

}  // namespace task
