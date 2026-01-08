#include "WorkThreadBase.h"
#include "common/LogHelper.h"
#include <atomic>
#include <cstdint>
#include <pthread.h>
#include "common/HETimerHelper.h"
#include "TaskQueueConstant.h"
namespace task
{
int32_t WorkThreadBase::sId()
{
    static std::atomic<int32_t> sId{ 1 << 16 };
    return sId.fetch_add(1, std::memory_order_relaxed);
}

void WorkThreadBase::_changeName()
{
    if (!mNameChanged)
    {
        return;
    }

#ifdef __APPLE__
    pthread_setname_np(mName.c_str());
#elif __ANDROID__
    pthread_setname_np(mThread.native_handle(), mName.c_str());
#endif

    mNameChanged = false;
}

void WorkThreadBase::_changePriority()
{
    if (!mPriorityChanged)
    {
        return;
    }

#ifdef __ANDROID__
    int pri = 0;
    switch (mPriority)
    {
        case task::WorkThreadPriority::WTP_Low:
            pri = 10;
            break;
        case task::WorkThreadPriority::WTP_Normal:
            pri = 0;
            break;
        case task::WorkThreadPriority::WTP_High:
            pri = -19;
            break;
        default:
            LOGE("[TASK][HY]unknown thread priority");
    }
    // int nice(int inc);  //  adds inc to the nice value for the calling thread
    int old = nice(0);
    int ret = nice(pri - old);
    if (-1 == ret)
    {
        LOGE("[TASK][HY]failed to set nice(%d), pri", ret);
    }
#else
    int                max  = sched_get_priority_max(SCHED_FIFO);
    int                min  = sched_get_priority_min(SCHED_FIFO);
    int                half = min + (max - min) / 2;
    struct sched_param sp;
    switch (mPriority)
    {
        case task::WorkThreadPriority::WTP_Low:
            sp.sched_priority = min;
            break;
        case task::WorkThreadPriority::WTP_High:
            sp.sched_priority = max;
            break;
        default:
            sp.sched_priority = half;
            break;
    }
    int error = pthread_setschedparam(mThread.native_handle(), SCHED_FIFO, &sp);
    if (error != 0)
    {
        LOGE("[TASK][HY]failed to set thread priority error: %d", error);
    }
#endif

    mPriorityChanged = false;
}

std::shared_ptr<IThreadPool> WorkThreadBase::_getThreadPool()
{
    if (!mThreadPool.expired())
    {
        return mThreadPool.lock();
    }
    return nullptr;
}

void WorkThreadBase::_updateStat(const TaskOperatorPtr& op)
{
    //收集统计信息
    std::lock_guard<std::mutex> lock(mStatMutex);
    auto                        timDif = task::HETimerHelper::currentTimeMillis() - mTaskStat.mLastStatTime;
    if (timDif > TaskQueueConstant::sOneMinuteMillisCount)
    {
        mTaskStat               = TaskStat{};
        mTaskStat.mLastStatTime = task::HETimerHelper::currentTimeMillis();
    }
    mTaskStat.mTaskCount++;
    mTaskStat.mAvgDuration += static_cast<int32_t>(op->taskRunDuration());
    mTaskStat.mPeakDuration = std::max(mTaskStat.mPeakDuration, static_cast<int32_t>(op->taskRunDuration()));
    mTaskStat.mAvgWaitDuration += static_cast<int32_t>(op->taskWaitDuration());
    mTaskStat.mPeakWaitDuration = std::max(mTaskStat.mPeakWaitDuration, static_cast<int32_t>(op->taskWaitDuration()));
}

std::string WorkThreadBase::_statInfo()
{
    std::lock_guard<std::mutex> lock(mStatMutex);
    std::string                 info = "tname: "
                       + mName
                       + " stat: avgR:"
                       + (mTaskStat.mTaskCount == 0 ? "0" : std::to_string(mTaskStat.mAvgDuration / mTaskStat.mTaskCount))
                       + " peakR:"
                       + std::to_string(mTaskStat.mPeakDuration)
                       + " avgW:"
                       + (mTaskStat.mTaskCount == 0 ? "0" : std::to_string(mTaskStat.mAvgWaitDuration / mTaskStat.mTaskCount))
                       + " peakW:"
                       + std::to_string(mTaskStat.mPeakWaitDuration);

    if (mCurrTask != nullptr)
    {
        auto currTask = std::atomic_load(&mCurrTask);
        if(currTask != nullptr)
        {
            info += " tskInf:" + currTask->taskCostInfo();
        }
    }
    return info;
}

}  // namespace task