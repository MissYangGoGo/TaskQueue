

#include "../TaskDispatch.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <stdio.h>
#include <thread>
using namespace task;
// clang++ -o test TestTaskQueueConcurrency.cpp -L../build/lib -ldispatch_queue -Wl,-rpath,@loader_path/../build/lib -g -O0

int main(int argc, char* argv[])
{

    // 同步任务
    printf("-------------------------------------- 同步任务 --------------------------------------\n");
    auto queueLow    = TaskQueueFactory::GetInstance().globalConcurrencyQueue(TaskQueuePriority::TQP_Low);
    auto queueNormal = TaskQueueFactory::GetInstance().globalConcurrencyQueue(TaskQueuePriority::TQP_Normal);
    auto queueHigh   = TaskQueueFactory::GetInstance().globalConcurrencyQueue(TaskQueuePriority::TQP_High);
    queueLow->async([]() {
        printf("test_execute 8: thread_id: %d\n", std::this_thread::get_id());
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    queueLow->async([]() {
        printf("test_execute 8-1: thread_id: %d\n", std::this_thread::get_id());
    });
    queueLow->async([]() {
        printf("test_execute 8-2: thread_id: %d\n", std::this_thread::get_id());
    });
    queueNormal->async([]() {
        printf("test_execute 9: thread_id: %d\n", std::this_thread::get_id());
    });
    queueNormal->async([]() {
        printf("test_execute 9-1: thread_id: %d\n", std::this_thread::get_id());
    });
    queueNormal->async([]() {
        printf("test_execute 9-2: thread_id: %d\n", std::this_thread::get_id());
    });
    queueHigh->async([]() {
        printf("test_execute 10: thread_id: %d\n", std::this_thread::get_id());
    });
    queueHigh->async([]() {
        printf("test_execute 10-1: thread_id: %d\n", std::this_thread::get_id());
    });
    queueHigh->async([]() {
        printf("test_execute 10-2: thread_id: %d\n", std::this_thread::get_id());
    });

    printf("-------------sync wait\n");

    std::this_thread::sleep_for(std::chrono::seconds(1));
    getchar();
    return 0;
}
