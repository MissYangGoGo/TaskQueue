#ifndef IThreadPool_H
#define IThreadPool_H

#include <cstdint>
#include <memory>
#include "TaskQueueDefine.h"
#include "Semaphore.h"
#include "QueueDefine.h"
namespace task
{
class WorkThreadBase;
class IThreadPool : public std::enable_shared_from_this<IThreadPool>
{
public:
    // 全局线程池
    static const std::shared_ptr<IThreadPool>& serialThreadPool();
    static const std::shared_ptr<IThreadPool>& parallelThreadPool();

public:
    virtual ~IThreadPool() = default;

    // 线程池管理线程生命周期
    virtual void registerWorkThread(const std::shared_ptr<WorkThreadBase>& /*thread*/)   = 0;
    virtual void unregisterWorkThread(const std::shared_ptr<WorkThreadBase>& /*thread*/) = 0;

    // 独占线程接口
    virtual void    execute(const TaskOperatorPtr& /*task*/, int32_t /*threadId*/) {}
    virtual int32_t attachOneThread(const std::string& /*name*/, WorkThreadPriority /*priority*/ = WorkThreadPriority::WTP_Normal)
    {
        return -1;
    }
    virtual void detachOneThread(int32_t /*threadID*/) {}

    // 全局线程池接口
    virtual void execute(const TaskOperatorPtr& /*task*/, TaskQueuePriority /*priority*/ = TaskQueuePriority::TQP_Normal) {}


    // 由线程判断 当前任务可能导致死锁时， 通知线程池切换另外一条线程执行
    virtual void notifyNextThread(int32_t /*threadID*/) {}

    // 线程池数据
    struct Data
    {
        Semaphore                                                  mSemaphore;              //信号量通知线程执行
        std::atomic<int32_t>                                       mMaxThreads{ 0 };        //线程池允许最大创建数
        std::atomic<int32_t>                                       mActiveThreads{ 0 };     //活动线程数
        std::atomic<int32_t>                                       mIdleThreads{ 0 };       //认为在wait等待的线程为idle线程
        std::array<WorkQueue, ( int )TaskQueuePriority::TQP_Count> mTaskQueues;             // 按优先级定义队列
        std::thread::id                                            mInputThreadID;          // 标记当前push任务的线程id (用于判断 push线程 和 执行线程不能在同一条线程进行)
    };
    virtual const std::shared_ptr<Data> getData() const
    {
        return nullptr;
    }
};

}  // namespace task

#endif  // IThreadPool_H