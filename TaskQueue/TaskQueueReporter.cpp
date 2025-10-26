#include "TaskQueueReporter.h"
// #include "common/report/HEReporterManager.hpp"
#ifdef __APPLE__
// #include "common/ThreadBacktraceHelper.h"
#endif

#include <unordered_map>
#include <string>
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
    std::unordered_map<std::string, std::string> extra{ { "threadCnt", std::move(msg) } };
    // comm::HEReporterManager::GetInstance().reportClient(comm::ReportKey::kThreadPool_Info, extra);
}

void TaskQueueReporter::_reportTaskDurationExceedThreshold(std::string&& msg)
{
    //上报任务超时
    std::unordered_map<std::string, std::string> extra{ { "taskBlock", std::move(msg) } };
    // comm::HEReporterManager::GetInstance().reportClient(comm::ReportKey::kThreadPool_Info, extra);
}

void TaskQueueReporter::_reportTaskCountExceedThreshold(std::string&& msg)
{
    // 任务数量超限
    std::unordered_map<std::string, std::string> extra{ { "taskExceed", std::move(msg) } };
    // comm::HEReporterManager::GetInstance().reportClient(comm::ReportKey::kThreadPool_Info, extra);
}

}  // namespace task