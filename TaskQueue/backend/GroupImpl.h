#ifndef __GROUP_IMPL_H__
#define __GROUP_IMPL_H__

#include "TaskQueueDefine.h"
#include "QueueDefine.h"
#include "Consumable.h"
#include <memory>
namespace task
{
class GroupImpl : public std::enable_shared_from_this<GroupImpl>
{
public:
    explicit GroupImpl(const ThreadPoolPtr& pool)
        : mThreadPool(pool)
        , mConsumable(std::make_shared<Consumable>(0)){};
    ~GroupImpl();

    // 在指定队列抛一个任务
    void async(const TaskOperatorPtr& task, const TaskQueuePtr& queue);

    // 在全局队列抛一个任务
    void async(const TaskOperatorPtr& task, TaskQueuePriority priority = TaskQueuePriority::TQP_Normal);

    // group所有执行完后，在指定queue上异步通知
    void notify(const TaskOperatorPtr& task, const TaskQueuePtr& queue);

    // group 等待所有任务结束
    bool wait(std::chrono::milliseconds t = std::chrono::milliseconds(-1));

private:
    ThreadPoolPtr mThreadPool;  //线程池
    ConsumablePtr mConsumable;
};

}  // namespace task

#endif  //__GROUP_IMPL_H__