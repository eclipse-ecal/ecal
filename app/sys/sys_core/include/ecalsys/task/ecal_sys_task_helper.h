#pragma once

#include <ecalsys/task/ecal_sys_task.h>
#include <sys_client_core/task.h>

namespace eCAL
{
  namespace sys
  {
    namespace task_helpers
    {
      eCAL::sys_client::Task                ToSysClientTask_NoLock           (const std::shared_ptr<EcalSysTask>& task);
      eCAL::sys_client::StartTaskParameters ToSysClientStartParameters_NoLock(const std::shared_ptr<EcalSysTask>& task);
      eCAL::sys_client::StopTaskParameters  ToSysClientStopParameters_NoLock (const std::shared_ptr<EcalSysTask>& task, bool by_name);
    }
  }
}