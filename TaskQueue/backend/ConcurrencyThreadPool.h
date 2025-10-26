#ifndef __TaskThreadPool_H__
#define __TaskThreadPool_H__

#include "IThreadPool.h"
#include "Semaphore.h"
#include <array>
#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "QueueDefine.h"
namespace task
{
class WorkThreadBase;
class ConcurrencyThreadPool : public IThreadPool
{
public:
    explicit ConcurrencyThreadPool();
    ~ConcurrencyThreadPool();

    // 线程池管理线程生命周期
    virtual void registerWorkThread(const std::shared_ptr<WorkThreadBase>& thread) override;
    virtual void unregisterWorkThread(const std::shared_ptr<WorkThreadBase>& thread) override;

    // 线程池管理任务
    virtual void execute(const TaskOperatorPtr& task, TaskQueuePriority priority = TaskQueuePriority::TQP_Normal) override;

    virtual void notifyNextThread(int32_t threadID) override;
    
    virtual const std::shared_ptr<Data> getData() const override
    {
        return mData;
    }

protected:
    // 线程调度
    void        _schedule(TaskQueuePriority priority);
    std::string _threadPoolInfo(std::string reason) const;

    void _monitorTask(TaskQueuePriority priority);

private:
    // 并行线程集合
    std::shared_ptr<Data>                                        mData;
    std::mutex                                                   mParallelMutex;
    std::unordered_map<int32_t, std::shared_ptr<WorkThreadBase>> mParallelThreads;
    std::vector<std::shared_ptr<WorkThreadBase>>                 mExpiredThreads;  // 过期线程

    // stat
    int32_t mReportCnt{ 0 };
};

}  // namespace task

#endif  // __TaskThreadPool_H__