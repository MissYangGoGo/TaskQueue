#include "../TaskDispatch.h"
#include "../TaskOperator.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <stdio.h>
#include <thread>
using namespace task;

// clang++ -o test TestTaskOperator.cpp -L../build/lib -ldispatch_queue -Wl,-rpath,@loader_path/../build/lib -g -O0

int main(int argc, char* argv[])
{
    printf("-------------------------------------- TaskOperator测试 --------------------------------------\n");
    
    // 测试基本任务操作
    auto task = std::make_shared<TaskOperator>([](const TaskOperatorPtr& t) {
        printf("TaskOperator测试任务执行: thread_id: %lu\n", std::this_thread::get_id());
    });
    
    assert(task != nullptr);
    printf("TaskOperator对象创建成功\n");
    
    // 测试用户数据设置和获取
    auto userData = std::make_shared<std::string>("test_data");
    task->setUserData(userData);
    
    auto retrievedData = task->userData<std::string>();
    assert(retrievedData != nullptr);
    assert(*retrievedData == "test_data");
    printf("用户数据设置和获取测试通过: %s\n", retrievedData->c_str());
    
    // 测试任务执行
    (*task)();
    printf("任务执行测试通过\n");
    
    // 测试任务取消功能
    auto cancellableTask = std::make_shared<TaskOperator>([](const TaskOperatorPtr& t) {
        if (t->isCancelled()) {
            printf("任务在执行前被取消\n");
        } else {
            printf("任务执行成功\n");
        }
    });
    
    assert(!cancellableTask->isCancelled());
    printf("任务初始状态为未取消\n");
    
    cancellableTask->cancel();
    assert(cancellableTask->isCancelled());
    printf("任务取消功能测试通过\n");
    
    (*cancellableTask)();
    printf("任务取消执行测试通过\n");
    
    // 测试任务时间记录功能
    auto timedTask = std::make_shared<TaskOperator>([](const TaskOperatorPtr& t) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        printf("带时间记录的任务执行完成\n");
    });
    
    timedTask->resetCallStartTime();
    (*timedTask)();
    
    printf("任务耗时信息: %s\n", timedTask->taskCostInfo().c_str());
    printf("任务执行时间: %llu ms\n", timedTask->taskRunDuration());
    printf("任务等待时间: %llu ms\n", timedTask->taskWaitDuration());
    printf("任务时间记录测试通过\n");
    
    printf("-------------TaskOperator测试完成-------------\n");
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    getchar();
    return 0;
}