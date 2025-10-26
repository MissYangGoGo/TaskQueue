
#ifndef WORK_THREAD_BASE_H
#define WORK_THREAD_BASE_H

#include <atomic>
#include <cstdint>
#include <string>
#include <thread>
#include <memory>
#include "QueueDefine.h"
#include "IThreadPool.h"
#include "TaskOperator.h"
namespace task
{
class WorkThreadBase : public std::enable_shared_from_this<WorkThreadBase>
{
public:
    explicit WorkThreadBase(const std::weak_ptr<IThreadPool>& threadPool)
        : mId(sId()), mThreadPool(threadPool) {}

    virtual ~WorkThreadBase() = default;

    // 获取线程ID
    int32_t threadId() const
    {
        return mId;
    }

    inline void setPriority(WorkThreadPriority prio)
    {
        mPriority        = prio;
        mPriorityChanged = true;
    }

    inline WorkThreadPriority getPriority() const
    {
        return mPriority;
    }

    inline void setName(const std::string& name)
    {
        mName        = name;
        mNameChanged = true;
    }

    inline const std::string& getName() const
    {
        return mName;
    }

    inline bool isRunning() const
    {
        return mIsRunning;
    }

    inline void cancel()
    {
        mIsCancelled.store(true, std::memory_order_release);
    }

    // 独占线程接口
    // ///////////////////////////////////////////////////////////////////////////////
    virtual bool isActive() const
    {
        return false;
    }
    virtual void setActive(bool /*active*/){};

    // 提交任务
    virtual bool commit(const TaskOperatorPtr& /*task*/)
    {
        return false;
    };
    virtual bool commit(TaskOperatorPtr&& /*task*/) noexcept
    {
        return false;
    };

    // 线程卡顿检查
    virtual bool isBlocked() const
    {
        return false;
    };
    virtual std::string blockedInfo()
    {
        return "";
    }

protected:
    // 获取全局线程ID
    static int32_t sId();

    void                         _changeName();
    void                         _changePriority();
    std::shared_ptr<IThreadPool> _getThreadPool();

    void        _updateStat(const TaskOperatorPtr& op);
    std::string _statInfo();

protected:
    int32_t            mId{ 0 };             // 线程ID
    bool               mIsRunning{ false };  //标记是否正在run， 可以统计是否当前线程卡在了一个操作上
    bool               mPriorityChanged{ false };
    bool               mNameChanged{ false };
    std::atomic<bool>  mIsCancelled{ false };  //是否取消
    WorkThreadPriority mPriority{ WorkThreadPriority::WTP_Normal };
    std::string        mName;

    std::thread                mThread;
    std::weak_ptr<IThreadPool> mThreadPool;  //归属线程池, 弱引用

    // 统计信息
    struct TaskStat
    {
        int32_t mTaskCount{ 0 };         //任务数量
        int32_t mAvgDuration{ 0 };       //平均任务耗时
        int32_t mPeakDuration{ 0 };      //峰值任务耗时
        int32_t mAvgWaitDuration{ 0 };   //平均等待耗时
        int32_t mPeakWaitDuration{ 0 };  //峰值等待耗时
        int64_t mLastStatTime{ 0 };      //上一次统计时间（一分钟重置）
    } mTaskStat;
    std::mutex      mStatMutex;
    TaskOperatorPtr mCurrTask{ nullptr };  //当前执行的任务
};

}  // namespace task

#endif  // WORK_THREAD_BASE_H
