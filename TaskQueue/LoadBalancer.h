// 智能负载感知和调度算法
#ifndef __LOAD_BALANCER_H__
#define __LOAD_BALANCER_H__

#include <cstdint>
#include <memory>
#include <atomic>
#include <chrono>
#include <vector>
#include <algorithm>
#include "TaskQueueDefine.h"
#include "common/SysUtils.h"

namespace task
{

// 系统指标结构
struct SystemMetrics {
    double cpuUsage = 0.0;           // CPU使用率 (0.0 - 1.0)
    double memoryUsage = 0.0;        // 内存使用率 (0.0 - 1.0)
    double ioWaitRatio = 0.0;        // IO等待比例 (0.0 - 1.0)
    int32_t cpuCount = 0;            // CPU核心数
    int64_t totalMemory = 0;         // 总内存 (bytes)
    int64_t availableMemory = 0;     // 可用内存 (bytes)
};

// 队列压力指标
struct QueuePressureMetrics {
    size_t currentDepth = 0;         // 当前队列深度
    double growthRate = 0.0;         // 队列增长率 (tasks/second)
    double avgWaitTime = 0.0;        // 平均等待时间 (ms)
    double taskThroughput = 0.0;     // 任务吞吐量 (tasks/second)
    TaskQueuePriority priority = TaskQueuePriority::TQP_Normal;
};

// 线程效率指标
struct ThreadEfficiencyMetrics {
    double utilization = 0.0;        // 线程利用率 (0.0 - 1.0)
    double avgTaskDuration = 0.0;    // 平均任务执行时间 (ms)
    double idleRatio = 0.0;          // 空闲比例 (0.0 - 1.0)
    int32_t activeThreads = 0;       // 活跃线程数
    int32_t idleThreads = 0;         // 空闲线程数
};

// 负载趋势预测
enum class LoadTrend {
    Stable,     // 稳定
    Increasing, // 增加
    Decreasing, // 减少
    Spiky       // 突发
};

// 负载指标聚合
struct LoadMetrics {
    SystemMetrics system;
    QueuePressureMetrics queuePressure;
    ThreadEfficiencyMetrics threadEfficiency;
    LoadTrend trend = LoadTrend::Stable;
    std::chrono::steady_clock::time_point timestamp;
};

// 任务类型分类
enum class TaskType {
    CPUIntensive,   // CPU密集型
    IOIntensive,    // IO密集型
    Mixed,          // 混合型
    Unknown         // 未知
};

// 自适应阈值管理器
class AdaptiveThresholdManager {
public:
    AdaptiveThresholdManager() = default;
    
    // 获取线程创建阈值
    uint32_t getCreateThreshold(TaskQueuePriority priority, 
                               const ThreadEfficiencyMetrics& metrics) const {
        // 高优先级任务：更低的阈值，更快响应
        if (priority == TaskQueuePriority::TQP_High) {
            return std::max(1u, static_cast<uint32_t>(metrics.idleThreads * 0.3));
        }
        
        // 普通优先级：平衡响应性和资源利用
        if (priority == TaskQueuePriority::TQP_Normal) {
            return std::max(5u, static_cast<uint32_t>(metrics.idleThreads * 0.6));
        }
        
        // 低优先级：更高阈值，避免过度创建线程
        return std::max(10u, static_cast<uint32_t>(metrics.idleThreads * 0.8));
    }
    
    // 根据系统状态调整最大线程数
    int32_t getOptimalMaxThreads(const SystemMetrics& system) const {
        int32_t baseCores = system.cpuCount > 0 ? system.cpuCount : SysUtils::cpuCount();
        
        // CPU密集型任务：接近核心数
        if (system.cpuUsage > 0.8) {
            return baseCores;
        }
        
        // IO密集型任务：可以超过核心数
        if (system.ioWaitRatio > 0.5) {
            return static_cast<int32_t>(baseCores * 1.5);
        }
        
        // 混合任务：动态调整
        return static_cast<int32_t>(baseCores * (1.0 + system.ioWaitRatio * 0.5));
    }
    
    // 获取队列压力阈值
    size_t getQueuePressureThreshold(TaskQueuePriority priority, const SystemMetrics& system) const {
        // 基础阈值根据优先级调整
        size_t baseThreshold = 0;
        switch (priority) {
            case TaskQueuePriority::TQP_High:
                baseThreshold = 5;
                break;
            case TaskQueuePriority::TQP_Normal:
                baseThreshold = 20;
                break;
            case TaskQueuePriority::TQP_Low:
                baseThreshold = 50;
                break;
            default:
                baseThreshold = 20;
                break;
        }
        
        // 根据系统负载调整阈值
        if (system.memoryUsage > 0.8) {
            // 内存压力大时降低阈值
            baseThreshold = static_cast<size_t>(baseThreshold * 0.5);
        } else if (system.cpuUsage < 0.3) {
            // CPU空闲时提高阈值
            baseThreshold = static_cast<size_t>(baseThreshold * 1.5);
        }
        
        return std::max(baseThreshold, static_cast<size_t>(1));
    }
};

// 增强的负载分析器
class EnhancedLoadAnalyzer {
public:
    EnhancedLoadAnalyzer() 
        : mLastAnalysisTime(std::chrono::steady_clock::now())
        , mLastQueueDepths{0, 0, 0} {
        mSystemMetrics.cpuCount = SysUtils::cpuCount();
    }
    
    // 获取系统CPU使用率
    double getSystemCpuUsage() const {
        // 在实际实现中，这里应该调用系统API获取真实CPU使用率
        // 目前返回模拟值
        return 0.5; // 50% CPU使用率
    }
    
    // 获取内存压力
    double getMemoryPressure() const {
        // 在实际实现中，这里应该获取真实内存使用情况
        // 目前返回模拟值
        return 0.3; // 30% 内存使用率
    }
    
    // 分析队列压力
    QueuePressureMetrics analyzeQueuePressure(TaskQueuePriority priority,
                                            size_t currentDepth,
                                            size_t previousDepth,
                                            std::chrono::milliseconds timeDelta) const {
        QueuePressureMetrics metrics;
        metrics.priority = priority;
        metrics.currentDepth = currentDepth;
        
        if (timeDelta.count() > 0) {
            // 计算队列增长率
            int64_t depthChange = static_cast<int64_t>(currentDepth) - static_cast<int64_t>(previousDepth);
            metrics.growthRate = static_cast<double>(depthChange) / (timeDelta.count() / 1000.0);
            
            // 简化的平均等待时间估算
            metrics.avgWaitTime = currentDepth * 10.0; // 假设每个任务平均等待10ms
            
            // 任务吞吐量估算
            metrics.taskThroughput = previousDepth > 0 ? 
                (static_cast<double>(previousDepth) / (timeDelta.count() / 1000.0)) : 0.0;
        }
        
        return metrics;
    }
    
    // 分析线程效率
    ThreadEfficiencyMetrics analyzeThreadEfficiency(int32_t activeThreads, 
                                                  int32_t idleThreads,
                                                  double avgTaskDuration) const {
        ThreadEfficiencyMetrics metrics;
        metrics.activeThreads = activeThreads;
        metrics.idleThreads = idleThreads;
        metrics.avgTaskDuration = avgTaskDuration;
        
        int32_t totalThreads = activeThreads + idleThreads;
        if (totalThreads > 0) {
            metrics.utilization = static_cast<double>(activeThreads) / totalThreads;
            metrics.idleRatio = static_cast<double>(idleThreads) / totalThreads;
        }
        
        return metrics;
    }
    
    // 预测负载趋势
    LoadTrend predictLoadTrend(std::chrono::milliseconds lookAhead) const {
        // 简化的趋势预测
        // 在实际实现中，应该基于历史数据进行预测
        return LoadTrend::Stable;
    }
    
    // 获取完整的负载指标
    LoadMetrics getLoadMetrics(TaskQueuePriority priority,
                              size_t currentDepth,
                              int32_t activeThreads,
                              int32_t idleThreads,
                              double avgTaskDuration) {
        auto now = std::chrono::steady_clock::now();
        auto timeDelta = std::chrono::duration_cast<std::chrono::milliseconds>(now - mLastAnalysisTime);
        
        LoadMetrics metrics;
        metrics.timestamp = now;
        
        // 系统指标
        metrics.system.cpuUsage = getSystemCpuUsage();
        metrics.system.memoryUsage = getMemoryPressure();
        metrics.system.cpuCount = SysUtils::cpuCount();
        
        // 队列压力指标
        size_t previousDepth = mLastQueueDepths[static_cast<int>(priority)];
        metrics.queuePressure = analyzeQueuePressure(priority, currentDepth, previousDepth, timeDelta);
        
        // 线程效率指标
        metrics.threadEfficiency = analyzeThreadEfficiency(activeThreads, idleThreads, avgTaskDuration);
        
        // 负载趋势
        metrics.trend = predictLoadTrend(std::chrono::milliseconds(1000));
        
        // 更新历史数据
        mLastAnalysisTime = now;
        mLastQueueDepths[static_cast<int>(priority)] = currentDepth;
        
        return metrics;
    }
    
    // 判断是否需要创建新线程
    bool shouldCreateNewThread(const LoadMetrics& metrics,
                              uint32_t createThreshold,
                              int32_t maxThreads) const {
        // 多重条件判断
        bool queuePressureHigh = metrics.queuePressure.currentDepth > createThreshold;
        bool systemNotOverloaded = metrics.system.cpuUsage < 0.8 && metrics.system.memoryUsage < 0.8;
        bool threadCapacityAvailable = (metrics.threadEfficiency.activeThreads + 
                                       metrics.threadEfficiency.idleThreads) < maxThreads;
        bool trendIncreasing = metrics.trend == LoadTrend::Increasing || metrics.trend == LoadTrend::Spiky;
        
        return (queuePressureHigh && systemNotOverloaded && threadCapacityAvailable) || trendIncreasing;
    }
    
    // 判断是否需要优化现有线程
    bool shouldOptimizeExisting(const LoadMetrics& metrics) const {
        // 线程利用率过低且队列不繁忙时，考虑优化
        bool lowUtilization = metrics.threadEfficiency.utilization < 0.3;
        bool lowQueuePressure = metrics.queuePressure.currentDepth < 5;
        bool tooManyIdleThreads = metrics.threadEfficiency.idleThreads > metrics.threadEfficiency.activeThreads * 2;
        
        return lowUtilization && lowQueuePressure && tooManyIdleThreads;
    }

private:
    mutable std::chrono::steady_clock::time_point mLastAnalysisTime;
    mutable size_t mLastQueueDepths[static_cast<int>(TaskQueuePriority::TQP_Count)];
};

} // namespace task

#endif // __LOAD_BALANCER_H__