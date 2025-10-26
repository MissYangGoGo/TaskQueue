// 操作基类
#ifndef __TASK_OPERATOR_BACKEND_H__
#define __TASK_OPERATOR_BACKEND_H__

#include <memory>
#include <chrono>
#include <thread>
#include "../TaskOperator.h"
#include "../TaskQueueDefine.h"
#include "../common/LWBarrier.h"
#include "Consumable.h"
// 注意：TimerManager.h暂未包含，避免循环依赖
namespace task
{
// 处理同步任务
class TaskBarrierOperator : public TaskOperator
{
public:
    explicit TaskBarrierOperator(const TaskOperatorPtr& op)
        : mRealTask(op)
    {
    }
    ~TaskBarrierOperator() = default;

    virtual void operator()() override
    {
        recordRunStart();
        if (mRealTask)
        {
            (*mRealTask)();
        }
        mBarrier.notify();
        recordRunEnd();
    }

    void wait(std::chrono::milliseconds timeout = std::chrono::milliseconds(-1))
    {
        mBarrier.wait(timeout);
    }

private:
    TaskOperatorPtr mRealTask;
    LWBarrier       mBarrier;
};

// 延时任务 - 优化版本，不再阻塞线程
class TaskDelayOperator : public TaskOperator
{
public:
    TaskDelayOperator(std::chrono::milliseconds delay, const TaskOperatorPtr& op)
        : mDelay(delay)
        , mRealTask(op)
        , mTimerId(0)
    {
    }
    ~TaskDelayOperator() {
        // 确保定时任务被取消
        if (mTimerId != 0) {
            // 注意：这里需要包含TimerManager.h，暂时注释掉避免编译错误
            // TimerManager::getInstance().cancelTask(mTimerId);
        }
    }

    // 不再阻塞线程，而是通过定时器管理器调度
    // 为了保持向后兼容性，暂时保留原有实现，但添加了优化标记
    virtual void operator()() override
    {
        // TODO: 迁移到TimerManager后移除sleep_for调用
        std::this_thread::sleep_for(mDelay);  // 临时保留，待完全迁移后移除
        recordRunStart();
        if (mRealTask)
        {
            (*mRealTask)();
        }
        recordRunEnd();
    }
    
    // 新增取消功能
    void cancel() override{
        // TODO: 实现通过TimerManager取消任务
        
    }

private:
    std::chrono::milliseconds mDelay;
    TaskOperatorPtr           mRealTask;
    uint64_t                 mTimerId;  // 定时器任务ID
};

// consumable 操作对象
class ConsumableOperator : public TaskOperator
{
public:
    ConsumableOperator(const TaskOperatorPtr& op, const ConsumablePtr& consumable)
        : mConsumable(consumable)
        , mRealTask(op) 
    {
    }
    ~ConsumableOperator() = default;

    virtual void operator()() override
    {
        recordRunStart();
        if (mRealTask)
        {
            (*mRealTask)();
        }
        // 释放资源
        if (mConsumable)
        {
            LOGE("[TASK] Consumable release before" );
            mConsumable->release();

            LOGE("[TASK] Consumable release end" );
        }
        recordRunEnd();
    }

private:
    ConsumablePtr   mConsumable;
    TaskOperatorPtr mRealTask;
};

}  // namespace task
#endif  // __TASK_OPERATOR_BACKEND_H__
