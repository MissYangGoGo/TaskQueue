#include "GroupImpl.h"
#include "Consumable.h"
#include "TaskOperatorBackend.h"
#include "TaskQueue.h"
#include "TaskQueueFactory.h"
#include "TaskQueueDefine.h"
#include <atomic>
#include <cassert>
#include <memory>
#include "common/LogHelper.h"
namespace task
{
GroupImpl::~GroupImpl()
{
    LOGE("[TASK]~GroupImpl()\n");
}
// 在指定队列抛一个任务
void GroupImpl::async(const TaskOperatorPtr& task, const TaskQueuePtr& queue)
{
    mConsumable->retain();

    auto consumTask =
        std::make_shared<ConsumableOperator>(task, mConsumable);
    queue->async(consumTask);
}

// 在全局队列抛一个任务
void GroupImpl::async(const TaskOperatorPtr& task, TaskQueuePriority priority)
{
    mConsumable->retain();
    auto consumTask =
        std::make_shared<ConsumableOperator>(task, mConsumable);
    TaskQueueFactory::GetInstance().globalConcurrencyQueue(priority)->async(consumTask);
}

// group所有执行完后，在指定queue上异步通知
void GroupImpl::notify(const TaskOperatorPtr& task, const TaskQueuePtr& queue)
{
    // 在指定的queue上 抛一个等待任务
    const auto self = shared_from_this();
    queue->async([task, self, queue] {
        queue->async(task);  //通知回调
        self->wait();        //group 等待所有执行完成
    });
}



// group 等待所有任务结束
bool GroupImpl::wait(std::chrono::milliseconds t)
{
    // 每次wait把当前的Consumable换掉，只等待之前提交的任务，后续提交的不影响本次等待
    ConsumablePtr oldC;
    ConsumablePtr newC;
    do
    {
        oldC = std::atomic_load(&mConsumable);
        newC = std::make_shared<Consumable>(0, oldC);  //依赖链式创建
    } while (!std::atomic_compare_exchange_weak(&mConsumable, &oldC, newC));

    assert(oldC);
    assert(newC);

    // 只等待之前提交的任务
    return oldC->wait_for(t);
}
}  // namespace task