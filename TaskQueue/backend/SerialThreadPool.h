#ifndef EXCLUSIVE_THREAD_POOL_H
#define EXCLUSIVE_THREAD_POOL_H

#include "IThreadPool.h"
#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include "TaskQueueDefine.h"
#include "common/ThreadRWLock.hpp"
namespace task
{
class SerialThreadPool : public IThreadPool
{
public:
    explicit SerialThreadPool()
        : mExclusiveThreadsLock(task::ThreadRWLock::create()) {}
    virtual void registerWorkThread(const std::shared_ptr<WorkThreadBase>& thread) override;
    virtual void unregisterWorkThread(const std::shared_ptr<WorkThreadBase>& thread) override;

    virtual void execute(const TaskOperatorPtr& task, int32_t threadId) override;

    // 获取对应优先级的独占线程
    virtual int32_t attachOneThread(const std::string& name, WorkThreadPriority priority = WorkThreadPriority::WTP_Normal) override;
    virtual void    detachOneThread(int32_t threadID) override;

private:
    std::string _threadPoolInfo(std::string reason) const;
    int32_t     _activeThreadCount() const;

private:
    // 独占线程集合
    std::shared_ptr<task::ThreadRWLock>                          mExclusiveThreadsLock;
    std::unordered_map<int32_t, std::shared_ptr<WorkThreadBase>> mExclusiveThreads;
    std::vector<std::shared_ptr<WorkThreadBase>>                 mExpiredThreads;  //延迟生命周期到下一次execute
};

}  // namespace task

#endif  // EXCLUSIVE_THREAD_POOL_H