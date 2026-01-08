#include "ConcurrencyThreadPool.h"
#include "QueueDefine.h"
#include "WorkThreadBase.h"
#include "WorkThreadConcurrency.h"
#include "SysUtils.h"
#include "common/LogHelper.h"
#include "TaskQueueConstant.h"
#include "TaskQueueReporter.h"

#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <mutex>
#include <string>
namespace task
{

ConcurrencyThreadPool::ConcurrencyThreadPool()
{
    LOGE("[TASK]ConcurrencyThreadPool()\n");
    // 线程池最大线程数为cpu核数
    mData = std::make_shared<Data>();
    mData->mMaxThreads.store(SysUtils::cpuCount(), std::memory_order_release);
}

ConcurrencyThreadPool::~ConcurrencyThreadPool()
{
    LOGE("[TASK]~ConcurrencyThreadPool()\n");
    // 通知所有线程退出
    mData->mSemaphore.release(mData->mMaxThreads.load(std::memory_order_acquire));
}

void ConcurrencyThreadPool::registerWorkThread(const std::shared_ptr<WorkThreadBase>& thread)
{
    if (thread == nullptr)
    {
        LOGE("[TASK]ConcurrencyThreadPool::registerWorkThread, thread is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(mParallelMutex);
    mParallelThreads[thread->threadId()] = thread;

    // 计数加一
    mData->mActiveThreads.fetch_add(1, std::memory_order_release);
    // mData->mIdleThreads.fetch_add(1, std::memory_order_release); [因为线程启动了， 执行过程中会++， 所以此处会重叠++， 出现一条线程， idle值为2的情况]
    TaskQueueReporter::GetInstance().notifyReport(TaskQueueReporterType::TQRT_ThreadCountChanged,
                                                  _threadPoolInfo(std::string(" threadid: ") + std::to_string(thread->threadId()) + " created"));
}

void ConcurrencyThreadPool::unregisterWorkThread(const std::shared_ptr<WorkThreadBase>& thread)
{
    if (thread == nullptr)
    {
        LOGE("[TASK]ConcurrencyThreadPool::unregisterWorkThread, thread is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(mParallelMutex);
    mExpiredThreads.push_back(thread);
    mParallelThreads.erase(thread->threadId());

    // 计数减一
    mData->mActiveThreads.fetch_sub(1, std::memory_order_release);
    mData->mIdleThreads.fetch_sub(1, std::memory_order_release);  // 确保线程池空闲线程数减一
    TaskQueueReporter::GetInstance().notifyReport(TaskQueueReporterType::TQRT_ThreadCountChanged,
                                                  _threadPoolInfo(std::string(" threadid: ") + std::to_string(thread->threadId()) + " released"));
}

void ConcurrencyThreadPool::execute(const TaskOperatorPtr& task, TaskQueuePriority priority)
{

    LOGE("[Job] %s, task: %p, priority: %d , threadId: %d", __FUNCTION__, task.get(), priority, std::this_thread::get_id());

    // 优先级判断
    if (priority < TaskQueuePriority::TQP_Low || priority >= TaskQueuePriority::TQP_Count)
    {
        LOGE("[TASK]ConcurrencyThreadPool::execute, invalid priority: %d \n", priority);
        assert(false);
        return;
    }

    // 队列数量监控
    _monitorTask(priority);

    // 标记当前线程
    mData->mInputThreadID = std::this_thread::get_id();

    int32_t    prio     = static_cast<int32_t>(priority);
    const auto enqueued = mData->mTaskQueues[prio].enqueue(task);
    if (enqueued)
    {
        // 信号量唤醒线程
        // 此处需要处理，
        mData->mSemaphore.release();
    }

    // 线程调度
    _schedule(priority);
}

void ConcurrencyThreadPool::notifyNextThread(int32_t threadID)
{
    LOGE("[Job] ConcurrencyThreadPool::notifyNextThread, threadId: %d \n", threadID);
    _schedule(TaskQueuePriority::TQP_Normal);
    mData->mSemaphore.release();
}

void ConcurrencyThreadPool::_monitorTask(TaskQueuePriority priority)
{
    int32_t prio = static_cast<int32_t>(priority);
    //1. 监控任务队列数量
    if (mData->mTaskQueues[prio].size_approx() >= TaskQueueConstant::sMaxTaskQueueCount)
    {
        TaskQueueReporter::GetInstance().notifyReportLimit(TaskQueueConstant::sMaxReportCountThreshold,
                                                           mReportCnt,
                                                           TaskQueueReporterType::TQRT_TaskCountExceedThreshold,
                                                           _threadPoolInfo(std::string(" tsk_exceed, pri: ")
                                                                           + std::to_string(prio)
                                                                           + ", tsk_cnt: "
                                                                           + std::to_string(mData->mTaskQueues[prio].size_approx())));
    }
    else
    {
        mReportCnt = 0;
    }
}

void ConcurrencyThreadPool::_schedule(TaskQueuePriority priority)
{

    // 如果有idle线程，不需要调度
    // 如果当前队列数量小于10，暂不创建新线程
    const auto idleCount   = mData->mIdleThreads.load(std::memory_order_acquire);
    const auto activeCount = mData->mActiveThreads.load(std::memory_order_acquire);

    if (activeCount > 0 && idleCount > 0 && mData->mTaskQueues[static_cast<int32_t>(priority)].size_approx() < TaskQueueConstant::sMaxTaskQueueCount)
    {
        mReportCnt = 0;
        return;
    }

    // 是否需要创建新线程
    if (activeCount < mData->mMaxThreads.load(std::memory_order_acquire))
    {
        // 创建新线程，注册到线程池
        auto thread = std::make_shared<WorkThreadConcurrency>(shared_from_this());
        LOGE("[Job] ConcurrencyThreadPool::_schedule, create new thread, threadId: %d, idleThreads: %d, activeThreads: %d \n", thread->threadId(), idleCount, activeCount);

        registerWorkThread(std::static_pointer_cast<WorkThreadBase>(thread));
        
    }
    else
    {
        // 如果达到最大线程数量，就只有等待被调度了
        TaskQueueReporter::GetInstance().notifyReportLimit(TaskQueueConstant::sMaxReportCountThreshold,
                                                           mReportCnt,
                                                           TaskQueueReporterType::TQRT_ThreadCountChanged,
                                                           _threadPoolInfo("max_threads_arrived"));

        //检查线程是否有block
        std::lock_guard<std::mutex> lock(mParallelMutex);
        for (auto it = mParallelThreads.begin(); it != mParallelThreads.end();)
        {
            if (it->second->isBlocked())
            {
                TaskQueueReporter::GetInstance().notifyReport(TaskQueueReporterType::TQRT_TaskDurationExceedThreshold,
                                                              it->second->blockedInfo());
                it = mParallelThreads.erase(it);

                // 减少一条活跃线程
                mData->mActiveThreads.fetch_sub(1, std::memory_order_release);
                continue;
            }
            ++it;
        }
    }
    // 删除过期线程, 确保生命周期正确
    {
        std::vector<std::shared_ptr<WorkThreadBase>> expiredThreads;
        {
            std::lock_guard<std::mutex> lock(mParallelMutex);
            expiredThreads.swap(mExpiredThreads);
        }
    }
}

std::string ConcurrencyThreadPool::_threadPoolInfo(std::string reason) const
{
    std::string info = "ParallelPool: active: "
                       + std::to_string(mData->mActiveThreads.load(std::memory_order_relaxed))
                       + ", idle: "
                       + std::to_string(mData->mIdleThreads.load(std::memory_order_relaxed))
                       + ", max: "
                       + std::to_string(mData->mMaxThreads.load(std::memory_order_relaxed))
                       + ", reason: "
                       + reason;

    LOGE("[TASK] %s \n", info.c_str());
    return info;
}

}  // namespace task