#include "TaskQueueReporter.h"
#include <string>
#include "common/LogHelper.h"
namespace task
{

void TaskQueueReporter::notifyReport(TaskQueueReporterType type, std::string&& msg)
{
    switch (type)
    {
        case TaskQueueReporterType::TQRT_ThreadCountChanged:
            _reportThreadCountChanged(std::move(msg));
            break;
        case TaskQueueReporterType::TQRT_TaskDurationExceedThreshold:
            _reportTaskDurationExceedThreshold(std::move(msg));
            break;
        case TaskQueueReporterType::TQRT_TaskCountExceedThreshold:
            _reportTaskCountExceedThreshold(std::move(msg));
            break;
        default:
            break;
    }
}

void TaskQueueReporter::notifyReportLimit(int32_t limit, int32_t& val, TaskQueueReporterType type, std::string&& msg)
{
    if (val >= limit)
    {
        return;
    }
    val++;

    notifyReport(type, std::move(msg));
}

void TaskQueueReporter::_reportThreadCountChanged(std::string&& msg)
{
    // 线程数量变化
    // TODO
    LOGI("Thread count changed: %s", msg.c_str());
}

void TaskQueueReporter::_reportTaskDurationExceedThreshold(std::string&& msg)
{
    //上报任务超时
    // TODO
    LOGI("Task duration exceed threshold: %s", msg.c_str());
}

void TaskQueueReporter::_reportTaskCountExceedThreshold(std::string&& msg)
{
    // 任务数量超限
    // TODO
    LOGI("Task count exceed threshold: %s", msg.c_str());
}

}  // namespace task