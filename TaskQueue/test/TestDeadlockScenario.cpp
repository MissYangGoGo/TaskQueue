#include "../TaskDispatch.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <stdio.h>
#include <thread>
using namespace task;

// clang++ -o test TestDeadlockScenario.cpp -L../build/lib -ldispatch_queue -Wl,-rpath,@loader_path/../build/lib -g -O0

int main(int argc, char* argv[])
{
    printf("-------------------------------------- 死锁场景测试 --------------------------------------\n");
    
    auto& factory = TaskQueueFactory::GetInstance();
    
    // 演示潜在的死锁问题
    printf("-------------------- 潜在死锁场景演示 --------------------\n");
    printf("注意：以下代码仅用于演示，实际应用中应避免这种做法\n");
    
    // 创建一个独占线程的串行队列
    auto exclusiveQueue = factory.createSerialTaskQueue("exclusive_queue", WorkThreadPriority::WTP_Normal, true);
    assert(exclusiveQueue != nullptr);
    
    // 在实际应用中，以下代码可能导致死锁，因此我们只演示安全的解决方案
    
    // 安全解决方案1: 使用notify替代wait
    printf("-------------------- 安全解决方案1: 使用notify --------------------\n");
    exclusiveQueue->async([&factory]() {
        printf("安全方案1 - 在队列任务中创建任务组, thread_id: %lu\n", std::this_thread::get_id());
        
        auto group = factory.createTaskGroup();
        assert(group != nullptr);
        
        group->async([]() {
            printf("安全方案1 - 任务组任务1执行, thread_id: %lu\n", std::this_thread::get_id());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
        
        group->async([]() {
            printf("安全方案1 - 任务组任务2执行, thread_id: %lu\n", std::this_thread::get_id());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
        
        // 使用notify而不是wait
        auto notifyQueue = factory.globalSerialQueue();
        group->notify([]() {
            printf("安全方案1 - 任务组所有任务完成通知, thread_id: %lu\n", std::this_thread::get_id());
        }, notifyQueue);
        
        printf("安全方案1 - 任务组已提交，使用异步通知避免死锁\n");
    });
    
    // 安全解决方案2: 使用不同的队列执行任务组任务
    printf("-------------------- 安全解决方案2: 使用不同队列 --------------------\n");
    auto sharedQueue = factory.createSerialTaskQueue("shared_queue", WorkThreadPriority::WTP_Normal, false);
    sharedQueue->async([&factory]() {
        printf("安全方案2 - 在队列任务中创建任务组, thread_id: %lu\n", std::this_thread::get_id());
        
        auto group = factory.createTaskGroup();
        assert(group != nullptr);
        
        // 指定不同的并发队列执行任务组任务
        auto concurrentQueue = factory.globalConcurrencyQueue(TaskQueuePriority::TQP_Normal);
        group->asyncQueue([]() {
            printf("安全方案2 - 任务组任务在并发队列执行, thread_id: %lu\n", std::this_thread::get_id());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }, concurrentQueue);
        
        group->asyncQueue([]() {
            printf("安全方案2 - 任务组任务在并发队列执行, thread_id: %lu\n", std::this_thread::get_id());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }, concurrentQueue);
        
        // 使用notify而不是wait
        auto notifyQueue = factory.globalSerialQueue();
        group->notify([]() {
            printf("安全方案2 - 任务组所有任务完成通知, thread_id: %lu\n", std::this_thread::get_id());
        }, notifyQueue);
        
        printf("安全方案2 - 任务组已提交到不同队列，使用异步通知避免死锁\n");
    });
    
    // 安全解决方案3: 使用同步任务避免死锁
    printf("-------------------- 安全解决方案3: 使用同步任务 --------------------\n");
    auto syncQueue = factory.globalConcurrencyQueue(TaskQueuePriority::TQP_Normal);
    syncQueue->sync([&factory]() {
        printf("安全方案3 - 在同步任务中创建任务组, thread_id: %lu\n", std::this_thread::get_id());
        
        auto group = factory.createTaskGroup();
        assert(group != nullptr);
        
        group->async([]() {
            printf("安全方案3 - 任务组任务执行, thread_id: %lu\n", std::this_thread::get_id());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
        
        // 在同步任务中，可以安全地使用wait，因为不会阻塞其他任务
        printf("安全方案3 - 等待任务组完成...\n");
        group->wait(std::chrono::milliseconds(5000)); // 设置超时避免永久等待
        printf("安全方案3 - 任务组完成\n");
    });
    
    printf("-------------死锁场景测试完成，请等待所有异步任务执行完毕-------------\n");
    
    // 等待所有异步任务完成
    std::this_thread::sleep_for(std::chrono::seconds(3));
    getchar();
    return 0;
}