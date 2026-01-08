#include "TaskOperator.h"
#include "common/HETimerHelper.h"
#include <string>
#include <thread>
namespace task
{

TaskOperator::TaskOperator()
{
    resetCallStartTime();
}

TaskOperator::TaskOperator(CallBack callback)
    : mCallBack(std::move(callback))
{
    resetCallStartTime();
}

void TaskOperator::operator()()
{
    if (mIsCancelled.load()) {
        // 任务已取消，不执行
        return;
    }
    
    recordRunStart();
    if (mCallBack)
    {
        mCallBack(shared_from_this());
    }
    recordRunEnd();
}

void TaskOperator::resetCallStartTime()
{
    mTaskCallStartTime = task::HETimerHelper::currentTimeMillis();
}

std::string TaskOperator::taskCostInfo() const
{
    return std::string("tskType:") + std::to_string(static_cast<int>(0))
           + std::string(" tskWait: ") + std::to_string(mTaskRunStartTime - mTaskCallStartTime)
           + std::string(" tskRun: ") + std::to_string(mTaskRunDuration);
}

uint64_t TaskOperator::taskRunDuration() const
{
    return mTaskRunDuration;
}

uint64_t TaskOperator::taskWaitDuration() const
{
    return mTaskRunStartTime - mTaskCallStartTime;
}

void TaskOperator::recordRunStart()
{
    mTaskRunStartTime = task::HETimerHelper::currentTimeMillis();
}
void TaskOperator::recordRunEnd()
{
    mTaskRunDuration = task::HETimerHelper::currentTimeMillis() - mTaskRunStartTime;
}

}  // namespace task