#include <sys_client_core/ecal_sys_client.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100 4127 4505 4800)
#endif
#include "ecal/pb/monitoring.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <ecal/ecal_process.h>
#include <ecal/ecal_util.h>

#include <chrono>
#include <EcalParser/EcalParser.h>
#include <ecal_utils/filesystem.h>

#include <sys_client_core/ecal_sys_client_logger.h>

namespace eCAL
{
  namespace sys_client
  {
    std::vector<int32_t> StartTasks(const std::vector<StartTaskParameters>& task_list)
    {
      auto now = std::chrono::system_clock::now();

      std::vector<int32_t> pid_list;
      pid_list.reserve(task_list.size());

      for (const auto& task_param : task_list)
      {
        Task task = task_param.task;
        EvaluateEcalParserFunctions(task, true, now);
        MergeRunnerIntoTask(task, EcalUtils::Filesystem::OsStyle::Current);

        int pid = eCAL::Process::StartProcess(task.path.c_str()
                                            , task.arguments.c_str()
                                            , task.working_dir.c_str()
                                            , task_param.create_console
                                            , task_param.window_mode
                                            , false);

        pid_list.push_back(static_cast<int32_t>(pid));

        if (pid != 0)
          EcalSysClientLogger::Instance()->info("Successfully started process (PID " + std::to_string(pid) + "): " + task.path + " " + task.arguments);
        else
          EcalSysClientLogger::Instance()->error("Failed to start Task: " + std::to_string(pid) + "): " + task.path + " " + task.arguments);
      }

      return pid_list;
    }

    std::vector<bool> StopTasks (const std::vector<StopTaskParameters>& task_list)
    {
      auto now = std::chrono::system_clock::now();

      std::vector<bool> success_list;
      success_list.reserve(task_list.size());

      for (const auto& task_param : task_list)
      {
        bool success = false;

        if (task_param.pid != 0)
        {
          // Stop by PID

          if (task_param.ecal_shutdown)
          {
            eCAL::Util::ShutdownProcess(task_param.pid);
            success = true;
          }
          else
          {
            success = eCAL::Process::StopProcess(task_param.pid);
          }
        }
        else
        {
          // Stop by name

          Task task = task_param.task;
          EvaluateEcalParserFunctions(task, true, now);
          MergeRunnerIntoTask(task, EcalUtils::Filesystem::OsStyle::Current);

          std::string filename = EcalUtils::Filesystem::FileName(task.path);
          
          if (!filename.empty())
          {
            if (task_param.ecal_shutdown)
            {
              eCAL::Util::ShutdownProcess(filename);
              success = true;
            }
            else
            {
              success = eCAL::Process::StopProcess(filename.c_str());
            }
          }
        }

        success_list.push_back(success);
      }

      return success_list;
    }

    std::vector<std::vector<int32_t>> MatchTasks(const std::vector<Task>& task_list)
    {
      std::vector<std::vector<int32_t>> list_of_pid_lists;

      // 1. Create a list of all evaluated Tasks. Just how we would start them.
      std::vector<Task> evaluated_task_list = task_list;
      auto now = std::chrono::system_clock::now();

      for (Task& task : evaluated_task_list)
      {
        EvaluateEcalParserFunctions(task, true, now);
        MergeRunnerIntoTask(task, EcalUtils::Filesystem::OsStyle::Current);
      }

      // 2. Get a list of all eCAL Tasks.
      std::string monitoring_string;
      eCAL::pb::Monitoring monitoring_pb;

      if (eCAL::Monitoring::GetMonitoring(monitoring_string))
      {
        monitoring_pb.ParseFromString(monitoring_string);
      }

      // 3. Check all tasks against all monitored processes and check if they match (by command line)
      for (const Task& evaluated_task : evaluated_task_list)
      {
        std::vector<int32_t> pid_list;
        for (const auto& process : monitoring_pb.processes())
        {
          if (process.hname() == eCAL::Process::GetHostName()) // Only handle local tasks!
          {
            if ((process.pparam() == evaluated_task.path)
              || (process.pparam() == "\"" + evaluated_task.path + "\"")
              || (process.pparam() == (evaluated_task.path + " " + evaluated_task.arguments))
              || (process.pparam() == ("\"" + evaluated_task.path + "\" " + evaluated_task.arguments)))
            {
              pid_list.push_back(process.pid());
            }
          }
        }

        list_of_pid_lists.push_back(pid_list);
      }

      return list_of_pid_lists;
    }
  }
}