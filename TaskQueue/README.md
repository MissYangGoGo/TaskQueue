# TaskQueue

一个基于 C++17 实现的高性能任务调度库，提供类似 iOS GCD (Grand Central Dispatch) 的任务队列功能，支持串行/并发任务执行、任务组同步、优先级调度等特性。

## 📋 目录

- [快速开始](#-快速开始)
- [核心概念](#-核心概念)
- [API 文档](#-api-文档)
- [使用示例](#-使用示例)
- [编译构建](#-编译构建)
- [项目结构](#-项目结构)
- [性能优化](#-性能优化)

## 🚀 快速开始

### 基本使用

```cpp
#include "TaskDispatch.h"
using namespace task;

int main() {
    // 获取工厂实例
    auto& factory = TaskQueueFactory::GetInstance();
    
    // 创建串行队列
    auto serialQueue = factory.createSerialTaskQueue(
        "my_serial_queue", 
        WorkThreadPriority::WTP_Normal, 
        false  // 非独占线程
    );
    
    // 异步执行任务
    serialQueue->async([]() {
        printf("异步任务执行\n");
    });
    
    // 同步执行任务
    serialQueue->sync([]() {
        printf("同步任务执行\n");
    });
    
    return 0;
}
```

### 使用任务组

```cpp
// 创建任务组
auto group = factory.createTaskGroup();

// 添加多个任务到组
group->async([]() {
    printf("任务1执行\n");
});

group->async([]() {
    printf("任务2执行\n");
});

// 异步等待所有任务完成
group->notify([]() {
    printf("所有任务完成\n");
});
```

## 📚 核心概念

### 1. 任务队列 (TaskQueue)

任务队列是任务调度的基本单元，分为以下类型：

#### 串行队列 (Serial Queue)
- **特点**：任务按 FIFO 顺序依次执行
- **适用场景**：需要保证执行顺序的任务，如文件 I/O、状态更新
- **线程模式**：
  - 共享线程模式：多个串行队列共享线程池
  - 独占线程模式：每个队列独占一个线程（适合长时间运行的任务）

#### 并发队列 (Concurrent Queue)
- **特点**：任务可以并发执行，不保证执行顺序
- **适用场景**：CPU 密集型计算、网络请求等可并行的任务
- **优先级**：支持高/中/低三级优先级

### 2. 任务组 (TaskGroup)

任务组用于管理一批相关任务的执行和同步：

- **批量提交**：一次性提交多个任务
- **等待完成**：`wait()` 阻塞等待所有任务完成
- **异步通知**：`notify()` 任务完成后回调
- **队列指定**：可以指定任务在特定队列执行

### 3. 任务操作 (TaskOperator)

任务的基本执行单元，支持：

- **取消操作**：`cancel()` 取消未执行的任务
- **用户数据**：`setUserData()` / `userData()` 携带任务数据
- **性能统计**：记录任务等待时间、执行时间

### 4. 线程池 (ThreadPool)

底层线程管理机制：

- **串行线程池**：管理串行队列的线程
- **并发线程池**：管理并发队列的线程，支持动态扩缩容
- **线程复用**：自动回收空闲线程，优化资源使用

## 📖 API 文档

### TaskQueueFactory

任务队列工厂类，单例模式，用于创建和获取队列。

```cpp
class TaskQueueFactory {
public:
    // 获取单例实例
    static TaskQueueFactory& GetInstance();
    
    // 创建串行队列
    // @param label: 队列名称
    // @param priority: 线程优先级
    // @param isExclusive: 是否独占线程
    TaskQueuePtr createSerialTaskQueue(
        const std::string& label, 
        WorkThreadPriority priority, 
        bool isExclusive
    );
    
    // 创建并发队列
    // @param label: 队列名称
    // @param priority: 任务优先级
    TaskQueuePtr createConcurrencyTaskQueue(
        const std::string& label, 
        TaskQueuePriority priority
    );
    
    // 获取全局并发队列（按优先级）
    TaskQueuePtr& globalConcurrencyQueue(TaskQueuePriority priority);
    
    // 获取全局串行队列
    TaskQueuePtr& globalSerialQueue();
    
    // 创建任务组
    TaskGroupPtr createTaskGroup();
};
```

### TaskQueue

任务队列类，提供任务调度接口。

```cpp
class TaskQueue {
public:
    // 异步执行任务
    void async(const TaskOperatorPtr& task);
    void async(std::function<void()>&& func);
    
    // 同步执行任务（阻塞等待）
    // @param timeout: 超时时间，默认无限等待
    void sync(
        const TaskOperatorPtr& task, 
        std::chrono::milliseconds timeout = std::chrono::milliseconds(-1)
    );
    void sync(
        std::function<void()>&& func,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(-1)
    );
    
    // 延时执行任务
    void after(
        std::chrono::milliseconds delay, 
        const TaskOperatorPtr& task
    );
    void after(
        std::chrono::milliseconds delay,
        std::function<void()>&& func
    );
    
    // 获取队列标签
    const std::string& label() const;
};
```

### TaskGroup

任务组类，用于批量管理任务。

```cpp
class TaskGroup {
public:
    // 在指定队列异步执行任务
    void asyncQueue(
        const TaskOperatorPtr& task, 
        const TaskQueuePtr& queue = nullptr
    );
    void asyncQueue(
        std::function<void()>&& func,
        const TaskQueuePtr& queue = nullptr
    );
    
    // 在全局队列异步执行任务
    void async(
        const TaskOperatorPtr& task, 
        TaskQueuePriority priority = TaskQueuePriority::TQP_Normal
    );
    void async(
        std::function<void()>&& func,
        TaskQueuePriority priority = TaskQueuePriority::TQP_Normal
    );
    
    // 任务组完成后通知
    void notify(
        const TaskOperatorPtr& task, 
        const TaskQueuePtr& queue = nullptr
    );
    void notify(
        std::function<void()>&& func,
        const TaskQueuePtr& queue = nullptr
    );
    
    // 等待所有任务完成
    // @param timeout: 超时时间，默认无限等待
    // @return: true-成功，false-超时
    bool wait(
        std::chrono::milliseconds timeout = std::chrono::milliseconds(-1)
    );
};
```

### TaskOperator

任务操作类，表示单个可执行任务。

```cpp
class TaskOperator {
public:
    using CallBack = std::function<void(const std::shared_ptr<TaskOperator>&)>;
    
    // 构造函数
    TaskOperator();
    explicit TaskOperator(CallBack callback);
    
    // 执行任务
    virtual void operator()();
    
    // 取消任务
    virtual void cancel();
    virtual bool isCancelled() const;
    
    // 用户数据
    void setUserData(const std::shared_ptr<void>& userData);
    
    template<typename T>
    std::shared_ptr<T> userData() const;
    
    // 性能统计
    uint64_t taskRunDuration() const;   // 任务执行时长
    uint64_t taskWaitDuration() const;  // 任务等待时长
    std::string taskCostInfo() const;   // 任务耗时信息
};
```

### 枚举类型

```cpp
// 队列类型
enum class TaskQueueType : uint8_t {
    TQT_Serial = 0,  // 串行队列
    TQT_Parallel,    // 并行队列
    TQT_Count,
};

// 任务优先级（并发队列）
enum class TaskQueuePriority : uint8_t {
    TQP_Low = 0,     // 低优先级
    TQP_Normal,      // 普通优先级
    TQP_High,        // 高优先级
    TQP_Count,
};

// 线程优先级（串行队列）
enum class WorkThreadPriority : uint8_t {
    WTP_Low = 0,     // 低优先级
    WTP_Normal,      // 普通优先级
    WTP_High,        // 高优先级
    WTP_Count,
};
```

## 💡 使用示例

### 示例 1：基本队列操作

```cpp
#include "TaskDispatch.h"
using namespace task;

void basicQueueExample() {
    auto& factory = TaskQueueFactory::GetInstance();
    
    // 创建串行队列
    auto serialQueue = factory.createSerialTaskQueue(
        "serial_queue", 
        WorkThreadPriority::WTP_Normal, 
        false
    );
    
    // 异步任务
    serialQueue->async([]() {
        printf("任务1执行\n");
    });
    
    serialQueue->async([]() {
        printf("任务2执行\n");
    });
    
    // 同步任务（会等待前面的任务完成）
    serialQueue->sync([]() {
        printf("同步任务执行\n");
    });
    
    // 延时任务
    serialQueue->after(std::chrono::seconds(1), []() {
        printf("1秒后执行\n");
    });
}
```

### 示例 2：并发队列与优先级

```cpp
void concurrentQueueExample() {
    auto& factory = TaskQueueFactory::GetInstance();
    
    // 获取不同优先级的全局并发队列
    auto highQueue = factory.globalConcurrencyQueue(TaskQueuePriority::TQP_High);
    auto normalQueue = factory.globalConcurrencyQueue(TaskQueuePriority::TQP_Normal);
    auto lowQueue = factory.globalConcurrencyQueue(TaskQueuePriority::TQP_Low);
    
    // 高优先级任务会优先执行
    for (int i = 0; i < 10; ++i) {
        highQueue->async([i]() {
            printf("高优先级任务 %d\n", i);
        });
    }
    
    // 普通优先级任务
    for (int i = 0; i < 10; ++i) {
        normalQueue->async([i]() {
            printf("普通优先级任务 %d\n", i);
        });
    }
    
    // 低优先级任务
    for (int i = 0; i < 10; ++i) {
        lowQueue->async([i]() {
            printf("低优先级任务 %d\n", i);
        });
    }
}
```

### 示例 3：任务组批量执行

```cpp
void taskGroupExample() {
    auto& factory = TaskQueueFactory::GetInstance();
    
    // 创建任务组
    auto group = factory.createTaskGroup();
    
    // 批量添加任务
    for (int i = 0; i < 5; ++i) {
        group->async([i]() {
            printf("任务 %d 开始执行\n", i);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            printf("任务 %d 执行完成\n", i);
        });
    }
    
    // 方式1：使用 notify 异步等待（推荐）
    group->notify([]() {
        printf("所有任务执行完成！\n");
    });
    
    // 方式2：使用 wait 阻塞等待
    // bool success = group->wait(std::chrono::seconds(5));
    // if (success) {
    //     printf("所有任务执行完成！\n");
    // } else {
    //     printf("等待超时！\n");
    // }
}
```

### 示例 4：任务取消

```cpp
void taskCancelExample() {
    auto& factory = TaskQueueFactory::GetInstance();
    auto queue = factory.globalSerialQueue();
    
    // 创建可取消的任务
    auto task = std::make_shared<TaskOperator>([](const TaskOperatorPtr& self) {
        if (self->isCancelled()) {
            printf("任务已被取消\n");
            return;
        }
        
        // 执行任务
        for (int i = 0; i < 10; ++i) {
            if (self->isCancelled()) {
                printf("任务执行中被取消\n");
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            printf("进度: %d/10\n", i + 1);
        }
        
        printf("任务完成\n");
    });
    
    // 提交任务
    queue->async(task);
    
    // 取消任务
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    task->cancel();
}
```

### 示例 5：独占线程队列

```cpp
void exclusiveThreadExample() {
    auto& factory = TaskQueueFactory::GetInstance();
    
    // 创建独占线程的串行队列
    // 适用于长时间运行的任务，如视频处理、文件监控等
    auto exclusiveQueue = factory.createSerialTaskQueue(
        "video_processing", 
        WorkThreadPriority::WTP_High,
        true  // 独占线程
    );
    
    exclusiveQueue->async([]() {
        printf("视频处理任务开始，线程ID: %lu\n", 
               std::this_thread::get_id());
        
        // 长时间运行的任务
        for (int i = 0; i < 100; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            printf("处理进度: %d%%\n", i + 1);
        }
        
        printf("视频处理完成\n");
    });
}
```

### 示例 6：任务数据传递

```cpp
struct TaskData {
    int id;
    std::string name;
    std::vector<int> values;
};

void taskDataExample() {
    auto& factory = TaskQueueFactory::GetInstance();
    auto queue = factory.globalConcurrencyQueue(TaskQueuePriority::TQP_Normal);
    
    // 创建带数据的任务
    auto task = std::make_shared<TaskOperator>([](const TaskOperatorPtr& self) {
        // 获取任务数据
        auto data = self->userData<TaskData>();
        if (data) {
            printf("处理任务: ID=%d, Name=%s\n", data->id, data->name.c_str());
            for (int val : data->values) {
                printf("  值: %d\n", val);
            }
        }
    });
    
    // 设置任务数据
    auto data = std::make_shared<TaskData>();
    data->id = 1001;
    data->name = "数据处理任务";
    data->values = {1, 2, 3, 4, 5};
    task->setUserData(data);
    
    // 执行任务
    queue->async(task);
}
```

### 示例 7：性能统计

```cpp
void performanceExample() {
    auto& factory = TaskQueueFactory::GetInstance();
    auto queue = factory.globalConcurrencyQueue(TaskQueuePriority::TQP_Normal);
    
    auto task = std::make_shared<TaskOperator>([](const TaskOperatorPtr& self) {
        // 模拟耗时操作
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // 输出性能统计
        printf("任务执行时间: %llu ms\n", self->taskRunDuration());
        printf("任务等待时间: %llu ms\n", self->taskWaitDuration());
        printf("%s\n", self->taskCostInfo().c_str());
    });
    
    queue->async(task);
}
```

## 🔨 编译构建

### 环境要求

- **C++ 编译器**：支持 C++17 标准（GCC 7+, Clang 5+, MSVC 2017+）
- **CMake**：3.14 或更高版本
- **平台**：macOS、iOS、Android

### 构建步骤

#### macOS / Linux

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译
make

# 编译产物在 build/lib 目录
```

#### iOS

```bash
mkdir build-ios && cd build-ios

# 配置 iOS 平台
cmake .. -DIOS=ON \
  -DCMAKE_TOOLCHAIN_FILE=path/to/ios.toolchain.cmake \
  -DPLATFORM=OS64

make
```

#### Android

```bash
mkdir build-android && cd build-android

# 使用 Android NDK
cmake .. -DANDROID=ON \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-21

make
```

### 测试

```bash
# 进入测试目录
cd test
mkdir build && cd build

# 配置测试
cmake ..

# 编译测试
make

# 运行测试
./TestTaskQueueFactory
./TestTaskQueue
./TestTaskGroup
./TestDeadlockScenario
# ... 更多测试
```

### 集成到项目

#### 方式 1：作为子模块

```bash
# 添加为 git 子模块
git submodule add <repository-url> third_party/TaskQueue
```

在你的 CMakeLists.txt 中：

```cmake
add_subdirectory(third_party/TaskQueue)

target_link_libraries(your_target PRIVATE dispatch_queue)
```

#### 方式 2：直接链接

```cmake
# 设置库路径
link_directories(/path/to/TaskQueue/build/lib)

# 链接库
target_link_libraries(your_target PRIVATE dispatch_queue)

# 添加头文件路径
target_include_directories(your_target PRIVATE /path/to/TaskQueue)
```

## 📁 项目结构

```
TaskQueue/
├── CMakeLists.txt              # 主构建配置
├── README.md                   # 本文档
├── TaskDispatch.h              # 统一头文件
├── TaskQueue.h/cpp             # 任务队列
├── TaskGroup.h/cpp             # 任务组
├── TaskQueueFactory.h/cpp      # 队列工厂
├── TaskOperator.h/cpp          # 任务操作
├── TaskQueueDefine.h           # 类型定义
├── TaskQueueConstant.h/cpp     # 常量定义
├── TaskQueueReporter.h/cpp     # 性能报告
├── LoadBalancer.h              # 负载均衡器
├── ObjectPool.h                # 对象池
│
├── backend/                    # 后端实现
│   ├── IThreadPool.h/cpp       # 线程池接口
│   ├── SerialThreadPool.h/cpp  # 串行线程池
│   ├── ConcurrencyThreadPool.h/cpp  # 并发线程池
│   ├── WorkThreadBase.h/cpp    # 工作线程基类
│   ├── WorkThreadSerial.h/cpp  # 串行工作线程
│   ├── WorkThreadConcurrency.h/cpp  # 并发工作线程
│   ├── IQueueImpl.h            # 队列实现接口
│   ├── SerialQueueImpl.h/cpp   # 串行队列实现
│   ├── ConcurrencyQueueImpl.h/cpp  # 并发队列实现
│   ├── GroupImpl.h/cpp         # 任务组实现
│   ├── TaskQueueFactoryImpl.h/cpp  # 工厂实现
│   └── QueueDefine.h           # 队列定义
│
├── common/                     # 通用工具
│   ├── Semaphore.h/cpp         # 信号量
│   ├── ThreadRWLock.h/cpp      # 读写锁
│   ├── LWBarrier.h/cpp         # 轻量级屏障
│   ├── HESingleton.h           # 单例模板
│   ├── HETimerHelper.h/cpp     # 定时器辅助
│   ├── SysUtils.h/cpp          # 系统工具
│   ├── LogHelper.h             # 日志辅助
│   ├── CppVersion.h            # C++ 版本检测
│   └── concurrentqueue.h       # 无锁并发队列（第三方）
│
└── test/                       # 测试代码
    ├── CMakeLists.txt          # 测试构建配置
    ├── TestTaskQueue.cpp       # 队列基础测试
    ├── TestTaskQueueFactory.cpp  # 工厂测试
    ├── TestTaskGroup.cpp       # 任务组测试
    ├── TestTaskOperator.cpp    # 任务操作测试
    ├── TestTaskQueueConcurrency.cpp  # 并发测试
    ├── TestTaskQueueComprehensive.cpp  # 综合测试
    ├── TestTaskGroupComprehensive.cpp  # 任务组综合测试
    ├── TestTaskQueueGroupIntegration.cpp  # 集成测试
    └── TestDeadlockScenario.cpp  # 死锁场景测试
```

## ⚡ 性能优化

### 1. 无锁并发队列

使用 [moodycamel::ConcurrentQueue](https://github.com/cameron314/concurrentqueue) 作为底层队列实现，提供：

- **高吞吐量**：支持多生产者多消费者模型
- **低延迟**：无锁设计，避免线程竞争
- **内存高效**：块分配策略，减少内存碎片

### 2. 线程池优化

- **动态线程管理**：根据任务负载自动创建/销毁线程
- **线程复用**：空闲线程自动回收，避免频繁创建
- **负载均衡**：任务均匀分配到各个线程
- **优先级调度**：高优先级任务优先执行

### 3. 内存优化

- **对象池**：复用任务对象，减少内存分配
- **智能指针**：自动内存管理，避免内存泄漏
- **块分配**：批量分配内存，提高效率

### 4. 性能监控

```cpp
// 获取任务性能数据
auto task = std::make_shared<TaskOperator>([](const TaskOperatorPtr& self) {
    // 任务逻辑
});

queue->async(task);

// 稍后查询性能
uint64_t waitTime = task->taskWaitDuration();  // 等待时长（ms）
uint64_t runTime = task->taskRunDuration();    // 执行时长（ms）
std::string info = task->taskCostInfo();       // 详细信息
```

## 🙏 致谢

- [moodycamel::ConcurrentQueue](https://github.com/cameron314/concurrentqueue) - 高性能无锁并发队列
- Apple GCD - 设计灵感来源

---

**⭐ 如果这个项目对你有帮助，请给一个 Star！**
