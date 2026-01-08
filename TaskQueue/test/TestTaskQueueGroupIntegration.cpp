#include "../TaskDispatch.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <stdio.h>
#include <thread>
using namespace task;

// clang++ -o test TestTaskQueueGroupIntegration.cpp -L../build/lib -ldispatch_queue -Wl,-rpath,@loader_path/../build/lib -g -O0

int main(int argc, char* argv[])
{
    printf("-------------------------------------- TaskQueue和TaskGroup集成测试 --------------------------------------\n");
    
    auto& factory = TaskQueueFactory::GetInstance();
    
    // 测试在队列中使用任务组
    printf("-------------------- 在队列中使用任务组测试 --------------------\n");
    auto queue = factory.createSerialTaskQueue("integration_queue", WorkThreadPriority::WTP_Normal, false);
    assert(queue != nullptr);
    
    queue->async([&factory]() {
        printf("在队列任务中创建任务组, thread_id: %lu\n", std::this_thread::get_id());
        
        auto group = factory.createTaskGroup();
        assert(group != nullptr);
        
        group->async([]() {
            printf("队列中任务组任务1执行, thread_id: %lu\n", std::this_thread::get_id());
        });
        
        group->async([]() {
            printf("队列中任务组任务2执行, thread_id: %lu\n", std::this_thread::get_id());
        });
        
        // 使用notify替代wait避免死锁
        auto notifyQueue = TaskQueueFactory::GetInstance().globalSerialQueue();
        group->notify([]() {
            printf("队列中任务组执行完成（通过notify通知）\n");
        }, notifyQueue);
        
        printf("队列中任务组已提交，使用notify等待完成...\n");
    });
    
    // 演示潜在的死锁问题及解决方案
    printf("-------------------- 死锁问题演示及解决方案 --------------------\n");
    auto deadlockQueue = factory.createSerialTaskQueue("deadlock_queue", WorkThreadPriority::WTP_Normal, true);
    assert(deadlockQueue != nullptr);
    
    // 不安全的做法（可能导致死锁）- 仅用于演示
    /*
    deadlockQueue->async([&factory]() {
        printf("潜在死锁演示 - 不安全做法, thread_id: %lu\n", std::this_thread::get_id());
        
        auto group = factory.createTaskGroup();
        group->async([]() {
            printf("任务组任务执行, thread_id: %lu\n", std::this_thread::get_id());
        });
        
        // 这里可能导致死锁，因为：
        // 1. 串行队列在独占线程中执行此任务
        // 2. 任务组中的任务可能需要在同一线程中执行
        // 3. wait()会阻塞当前线程，导致任务无法执行
        // group->wait(); // 不要这样做！
        
        printf("注意：此处避免了wait()调用以防止死锁\n");
    });
    */
    
    // 安全的做法
    deadlockQueue->async([&factory]() {
        printf("死锁安全解决方案演示, thread_id: %lu\n", std::this_thread::get_id());
        
        auto group = factory.createTaskGroup();
        assert(group != nullptr);
        
        group->async([]() {
            printf("安全方案任务组任务1执行, thread_id: %lu\n", std::this_thread::get_id());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
        
        group->async([]() {
            printf("安全方案任务组任务2执行, thread_id: %lu\n", std::this_thread::get_id());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
        
        // 解决方案1: 使用notify而不是wait
        auto completionQueue = TaskQueueFactory::GetInstance().globalConcurrencyQueue(TaskQueuePriority::TQP_Normal);
        group->notify([]() {
            printf("安全方案任务组完成通知, thread_id: %lu\n", std::this_thread::get_id());
        }, completionQueue);
        
        printf("安全方案任务组已提交，使用异步通知避免死锁\n");
    });
    
    // 解决方案2: 在不同队列中执行任务组任务
    auto solutionQueue = factory.createSerialTaskQueue("solution_queue", WorkThreadPriority::WTP_Normal, false);
    solutionQueue->async([&factory]() {
        printf("死锁解决方案2演示, thread_id: %lu\n", std::this_thread::get_id());
        
        auto group = factory.createTaskGroup();
        assert(group != nullptr);
        
        // 指定不同的队列执行任务组任务
        auto taskQueue = TaskQueueFactory::GetInstance().globalConcurrencyQueue(TaskQueuePriority::TQP_Normal);
        group->asyncQueue([]() {
            printf("解决方案2任务组任务执行, thread_id: %lu\n", std::this_thread::get_id());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }, taskQueue);
        
        // 使用notify而不是wait
        auto notifyQueue = TaskQueueFactory::GetInstance().globalSerialQueue();
        group->notify([]() {
            printf("解决方案2任务组完成通知, thread_id: %lu\n", std::this_thread::get_id());
        }, notifyQueue);
        
        printf("解决方案2任务组已提交，使用异步通知避免死锁\n");
    });
    
    // 测试任务组中使用队列
    printf("-------------------- 在任务组中使用队列测试 --------------------\n");
    auto group = factory.createTaskGroup();
    assert(group != nullptr);
    
    auto customQueue1 = factory.createSerialTaskQueue("group_queue_1", WorkThreadPriority::WTP_Normal, false);
    auto customQueue2 = factory.createConcurrencyTaskQueue("group_queue_2", TaskQueuePriority::TQP_Normal);
    
    group->asyncQueue([&customQueue1]() {
        printf("任务组中使用自定义串行队列, thread_id: %lu\n", std::this_thread::get_id());
        
        customQueue1->async([]() {
            printf("自定义串行队列中的任务执行, thread_id: %lu\n", std::this_thread::get_id());
        });
    }, customQueue1);
    
    group->asyncQueue([&customQueue2]() {
        printf("任务组中使用自定义并发队列, thread_id: %lu\n", std::this_thread::get_id());
        
        customQueue2->async([]() {
            printf("自定义并发队列中的任务1执行, thread_id: %lu\n", std::this_thread::get_id());
        });
        
        customQueue2->async([]() {
            printf("自定义并发队列中的任务2执行, thread_id: %lu\n", std::this_thread::get_id());
        });
    }, customQueue2);
    
    printf("等待任务组完成...\n");
    group->wait();
    printf("任务组执行完成\n");
    
    // 测试复杂的嵌套使用场景
    printf("-------------------- 复杂嵌套使用场景测试 --------------------\n");
    auto complexQueue = factory.createConcurrencyTaskQueue("complex_queue", TaskQueuePriority::TQP_Normal);
    assert(complexQueue != nullptr);
    
    complexQueue->async([&factory]() {
        printf("复杂场景任务1开始, thread_id: %lu\n", std::this_thread::get_id());
        
        auto subGroup = factory.createTaskGroup();
        subGroup->async([]() {
            printf("复杂场景子任务组任务1执行, thread_id: %lu\n", std::this_thread::get_id());
        });
        
        subGroup->async([]() {
            printf("复杂场景子任务组任务2执行, thread_id: %lu\n", std::this_thread::get_id());
        });
        
        subGroup->wait();
        printf("复杂场景子任务组完成\n");
    });
    
    complexQueue->async([&factory]() {
        printf("复杂场景任务2开始, thread_id: %lu\n", std::this_thread::get_id());
        
        auto anotherQueue = factory.createSerialTaskQueue("another_queue", WorkThreadPriority::WTP_Normal, true);
        anotherQueue->async([]() {
            printf("复杂场景另一个队列任务执行, thread_id: %lu\n", std::this_thread::get_id());
        });
    });
    
    // 测试同步和异步混合使用
    printf("-------------------- 同步和异步混合使用测试 --------------------\n");
    int syncResult = 0;
    auto syncQueue = factory.globalConcurrencyQueue(TaskQueuePriority::TQP_Normal);
    
    syncQueue->sync([&syncResult, &factory]() {
        printf("同步任务中创建任务组, thread_id: %lu\n", std::this_thread::get_id());
        
        auto syncGroup = factory.createTaskGroup();
        syncGroup->async([&syncResult]() {
            syncResult = 42;
            printf("同步任务组中修改结果值为: %d, thread_id: %lu\n", syncResult, std::this_thread::get_id());
        });
        
        syncGroup->wait();
    });
    
    printf("同步任务执行后结果值: %d (期望值: 42)\n", syncResult);
    
    printf("-------------TaskQueue和TaskGroup集成测试完成，请等待所有异步任务执行完毕-------------\n");
    
    // 等待所有异步任务完成
    std::this_thread::sleep_for(std::chrono::seconds(3));
    getchar();
    return 0;
}