#ifndef WORK_THREAD_SERIAL_H
#define WORK_THREAD_SERIAL_H

#include "WorkThreadBase.h"

#include <cstdint>
#include <memory>
namespace task
{

class WorkThreadSerial : public WorkThreadBase
{
public:
    explicit WorkThreadSerial(const std::weak_ptr<IThreadPool>& threadPool);
    ~WorkThreadSerial();

    virtual bool isActive() const override
    {
        return mIsActive;
    }
    virtual void setActive(bool active) override;

    // 提交任务
    virtual bool        commit(const TaskOperatorPtr& task) override;
    virtual bool        commit(TaskOperatorPtr&& task) noexcept override;
    virtual bool        isBlocked() const override;
    virtual std::string blockedInfo() override;

protected:
    void _run();
    void _exclusive();
    void _monitorTask();

private:
    bool      mIsActive{ false };  //是否为活跃线程
    WorkQueue mWorkQueue;          //线程任务队列
    Semaphore mSemaphore;          // 独占线程通知
    int32_t   mReportCount{ 0 };
    int64_t   mStartRunTime{ 0 };
};

}  // namespace task

#endif  // WORK_THREAD_SERIAL_H
