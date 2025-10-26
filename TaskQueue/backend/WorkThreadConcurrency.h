#ifndef WORK_THREAD_PARALLEL_H
#define WORK_THREAD_PARALLEL_H

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>
#include <mutex>
#include "QueueDefine.h"
#include "IThreadPool.h"
#include "TaskQueueDefine.h"
#include "WorkThreadBase.h"
namespace task
{
class WorkThreadConcurrency : public WorkThreadBase
{
public:
    explicit WorkThreadConcurrency(const std::weak_ptr<IThreadPool>& threadPool);
    ~WorkThreadConcurrency();

    // 线程卡顿检查
    virtual bool        isBlocked() const override;
    virtual std::string blockedInfo() override;

protected:
    void _run();
    bool _parallel();

private:
    int64_t mStartRunTime{ 0 };
    std::thread::id mNativeThreadId;
};

}  // namespace task

#endif  // WORK_THREAD_PARALLEL_H
