// 串行队列实现
#ifndef __SERIAL_TASK_QUEUE_IMPL_H__
#define __SERIAL_TASK_QUEUE_IMPL_H__

#include "IQueueImpl.h"
#include "QueueDefine.h"
#include "TaskQueueDefine.h"
namespace task
{
class SerialQueueImpl : public IQueueImpl
{
public:
    SerialQueueImpl(const std::string& label, bool isExclusive, const ThreadPoolPtr& threadPool, WorkThreadPriority prio);
    ~SerialQueueImpl();

    virtual void async(const TaskOperatorPtr& task) override;
    virtual void sync(const TaskOperatorPtr& task, std::chrono::milliseconds timeout = std::chrono::milliseconds(-1)) override;
    virtual void after(std::chrono::milliseconds delay, const TaskOperatorPtr& task) override;

private:
    void _processTask();

private:
    bool                mIsExclusive{ false };
    int32_t             mThreadId{ -1 };
    std::atomic_flag    mSyncFlag;   // 同步标志位

    WorkQueue           mTasks;         // 串行任务队列
    TaskOperatorPtr     mSerialTask;    // 当前正在执行的任务

};

}  // namespace task

#endif  // __SERIAL_TASK_QUEUE_IMPL_H__