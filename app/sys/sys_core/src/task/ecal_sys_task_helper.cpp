#include <ecalsys/task/ecal_sys_task_helper.h>

namespace eCAL
{
  namespace sys
  {
    namespace task_helpers
    {
      eCAL::sys_client::Task ToSysClientTask_NoLock(const std::shared_ptr<EcalSysTask>& task)
      {
        eCAL::sys_client::Task client_task;

        if (task->GetRunner())
        {
          client_task.runner.path             = task->GetRunner()->GetPath();
          client_task.runner.arguments        = task->GetRunner()->GetLoadCmdArgument();
          client_task.runner.default_task_dir = task->GetRunner()->GetDefaultAlgoDir();
        }
        client_task.path        = task->GetAlgoPath();
        client_task.arguments   = task->GetCommandLineArguments();
        client_task.working_dir = task->GetWorkingDir();

        return client_task;
      }

      eCAL::sys_client::StartTaskParameters ToSysClientStartParameters_NoLock(const std::shared_ptr<EcalSysTask>& task)
      {
        eCAL::sys_client::StartTaskParameters start_task_param;

        start_task_param.create_console = false;
        start_task_param.window_mode    = task->GetVisibility();
        start_task_param.task           = ToSysClientTask_NoLock(task);

        return start_task_param;
      }

      eCAL::sys_client::StopTaskParameters ToSysClientStopParameters_NoLock(const std::shared_ptr<EcalSysTask>& task, bool by_name)
      {
        eCAL::sys_client::StopTaskParameters stop_task_param;

        if (by_name)
        {
          stop_task_param.task             = ToSysClientTask_NoLock(task);
          stop_task_param.ecal_shutdown    = true;
        }
        else if (!task->GetPids().empty() && (task->GetPids()[0] != 0))
        {
          stop_task_param.process_id           = task->GetPids()[0];
          stop_task_param.ecal_shutdown = true;
        }

        return stop_task_param;
      }
    }
  }
}
