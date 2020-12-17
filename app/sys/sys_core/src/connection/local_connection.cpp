#include "local_connection.h"

#include <sys_client_core/ecal_sys_client.h>

#include <ecal/ecal_process.h>

namespace eCAL
{
  namespace sys
  {

    LocalConnection::LocalConnection()
      : AbstractConnection(eCAL::Process::GetHostName())
    {}

    LocalConnection::~LocalConnection()
    {}

    std::vector<int32_t> LocalConnection::StartTasks(const std::vector<eCAL::sys_client::StartTaskParameters>& task_list)
    {
      std::lock_guard<decltype(connection_mutex_)> connection_lock(connection_mutex_);
      return eCAL::sys_client::StartTasks(task_list);
    }

    std::vector<bool> LocalConnection::StopTasks (const std::vector<eCAL::sys_client::StopTaskParameters>& task_list)
    {
      std::lock_guard<decltype(connection_mutex_)> connection_lock(connection_mutex_);
      return eCAL::sys_client::StopTasks(task_list);
    }

    std::vector<std::vector<int32_t>> LocalConnection::MatchTasks(const std::vector<eCAL::sys_client::Task>& task_list)
    {
      std::lock_guard<decltype(connection_mutex_)> connection_lock(connection_mutex_);
      return eCAL::sys_client::MatchTasks(task_list);
    }
  }
}