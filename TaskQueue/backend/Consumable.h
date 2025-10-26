// 可消费对象
// 用于记录 group的 任务计数
// 可以链式等待
#ifndef __CONSUMABLE_H__
#define __CONSUMABLE_H__

#include <atomic>
#include "LogHelper.h"
#include "common/LWBarrier.h"
namespace task
{
class Consumable;
using ConsumablePtr = std::shared_ptr<Consumable>;

class Consumable
{
public:
    explicit Consumable(int32_t resources = 0, const ConsumablePtr& depConsumable = nullptr)
        : mResources(resources)
        , mDepConsumable(depConsumable) {}
    ~Consumable() = default;

    // 添加一个资源
    void retain()
    {
        mResources.fetch_add(1);
    }

    // 消费一个资源
    void release()
    {
        // 异常处理，避免负数产生
        if(mResources.load(std::memory_order_relaxed) == 0)
        {
            return;
        }

        // 如果资源数为0，通知等待的线程
        if (1 == mResources.fetch_sub(1))
        {
            mBarrier.notify();
        }
    }

    // 等待
    bool wait_for(std::chrono::milliseconds timeout = std::chrono::milliseconds(-1))
    {   
        // 判断依赖
        if (mDepConsumable && mDepConsumable.get() != this)
        {
            // 超时返回false
            if (!mDepConsumable->wait_for(timeout))
            {
                return false;
            }
        }

        // 当前没有资源等待
        if (mResources.load() == 0)
        {
            return true;
        }

        // 等待资源
        return mBarrier.wait(timeout);
    }

private:
    std::atomic<int32_t> mResources;  //记录资源数
    LWBarrier            mBarrier;

    const ConsumablePtr mDepConsumable;  //依赖的可消费对象
};

}  // namespace task

#endif  // __CONSUMABLE_H__