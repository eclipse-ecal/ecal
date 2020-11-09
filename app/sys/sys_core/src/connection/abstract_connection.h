#pragma once

#include <string>
#include <vector>

#include <sys_client_core/task.h>

namespace eCAL
{
  namespace sys
  {
    class AbstractConnection
    {
    public:
      AbstractConnection(const std::string& hostname)
        : m_hostname(hostname)
      {}

      virtual ~AbstractConnection() {};

      virtual std::vector<int32_t>              StartTasks(const std::vector<eCAL::sys_client::StartTaskParameters>& task_list) = 0;

      virtual std::vector<bool>                 StopTasks (const std::vector<eCAL::sys_client::StopTaskParameters>&  task_list) = 0;

      virtual std::vector<std::vector<int32_t>> MatchTasks(const std::vector<eCAL::sys_client::Task>&                task_list) = 0;

    protected:
      const std::string m_hostname;
    };
  }
}