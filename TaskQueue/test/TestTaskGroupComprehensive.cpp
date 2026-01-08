#include "../TaskDispatch.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <stdio.h>
#include <thread>
using namespace task;

// clang++ -o test TestTaskGroupComprehensive.cpp -L../build/lib -ldispatch_queue -Wl,-rpath,@loader_path/../build/lib -g -O0

int main(int argc, char* argv[])
{
    printf("-------------------------------------- TaskGroup综合功能测试 --------------------------------------\n");
    
    auto& factory = TaskQueueFactory::GetInstance();
    
    // 测试基本任务组功能
    printf("-------------------- 基本任务组功能测试 --------------------\n");
    auto group = factory.createTaskGroup();
    assert(group != nullptr);
    printf("任务组创建成功\n");
    
    int groupCounter = 0;
    group->async([&groupCounter]() {
        groupCounter++;
        printf("任务组任务1执行, counter: %d, thread_id: %lu\n", groupCounter, std::this_thread::get_id());
    });
    
    group->async([&groupCounter]() {
        groupCounter++;
        printf("任务组任务2执行, counter: %d, thread_id: %lu\n", groupCounter, std::this_thread::get_id());
    });
    
    group->async([&groupCounter]() {
        groupCounter++;
        printf("任务组任务3执行, counter: %d, thread_id: %lu\n", groupCounter, std::this_thread::get_id());
    });
    
    printf("等待任务组所有任务完成...\n");
    group->wait();
    printf("任务组执行完成, 最终counter值: %d (期望值: 3)\n", groupCounter);
    
    // 测试指定队列的任务组功能
    printf("-------------------- 指定队列任务组功能测试 --------------------\n");
    auto customQueue = factory.createSerialTaskQueue("group_custom_queue", WorkThreadPriority::WTP_Normal, false);
    assert(customQueue != nullptr);
    
    auto group2 = factory.createTaskGroup();
    assert(group2 != nullptr);
    
    group2->asyncQueue([]() {
        printf("任务组指定队列任务1执行, thread_id: %lu\n", std::this_thread::get_id());
    }, customQueue);
    
    group2->asyncQueue([]() {
        printf("任务组指定队列任务2执行, thread_id: %lu\n", std::this_thread::get_id());
    }, customQueue);
    
    printf("等待指定队列任务组完成...\n");
    group2->wait();
    printf("指定队列任务组执行完成\n");
    
    // 测试任务组通知功能
    printf("-------------------- 任务组通知功能测试 --------------------\n");
    auto group3 = factory.createTaskGroup();
    assert(group3 != nullptr);
    
    group3->async([]() {
        printf("任务组通知测试任务1执行, thread_id: %lu\n", std::this_thread::get_id());
        // std::this_thread::sleep_for(std::chrono::milliseconds(1));
    });
    
    group3->async([]() {
        printf("任务组通知测试任务2执行, thread_id: %lu\n", std::this_thread::get_id());
        // std::this_thread::sleep_for(std::chrono::milliseconds(1));
    });
    
    auto notifyQueue = factory.globalSerialQueue();
    group3->notify([]() {
        printf("任务组所有任务完成后的通知执行, thread_id: %lu\n", std::this_thread::get_id());
    }, notifyQueue);
    
    printf("任务组通知已设置，等待任务完成...\n");
    
    // 测试不同优先级的任务组任务
    printf("-------------------- 不同优先级任务组任务测试 --------------------\n");
    auto group4 = factory.createTaskGroup();
    assert(group4 != nullptr);
    
    group4->async([]() {
        printf("任务组低优先级任务执行, thread_id: %lu\n", std::this_thread::get_id());
    }, TaskQueuePriority::TQP_Low);
    
    group4->async([]() {
        printf("任务组普通优先级任务执行, thread_id: %lu\n", std::this_thread::get_id());
    }, TaskQueuePriority::TQP_Normal);
    
    group4->async([]() {
        printf("任务组高优先级任务执行, thread_id: %lu\n", std::this_thread::get_id());
    }, TaskQueuePriority::TQP_High);
    
    printf("等待不同优先级任务组完成...\n");
    group4->wait();
    printf("不同优先级任务组执行完成\n");
    
    printf("-------------TaskGroup综合功能测试完成，请等待所有异步任务执行完毕-------------\n");
    
    // 等待所有异步任务完成
    std::this_thread::sleep_for(std::chrono::seconds(2));
    getchar();
    return 0;
}