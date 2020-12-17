#include "connection_manager.h"

#include <ecal/ecal_process.h>

namespace eCAL
{
  namespace sys
  {
    //////////////////////////////////
    // Constructor & Destructor
    //////////////////////////////////

    ConnectionManager::ConnectionManager()
    {}

    ConnectionManager::~ConnectionManager()
    {}

    //////////////////////////////////
    // API
    //////////////////////////////////

    std::vector<int32_t> ConnectionManager::StartTasks(const std::string& hostname, const std::vector<eCAL::sys_client::StartTaskParameters>& task_list)
    {
      return GetConnection(hostname)->StartTasks(task_list);
    }

    std::vector<bool> ConnectionManager::StopTasks (const std::string& hostname, const std::vector<eCAL::sys_client::StopTaskParameters>& task_list)
    {
      return GetConnection(hostname)->StopTasks(task_list);
    }

    std::vector<std::vector<int32_t>> ConnectionManager::MatchTasks(const std::string& hostname, const std::vector<eCAL::sys_client::Task>& task_list)
    {
      return GetConnection(hostname)->MatchTasks(task_list);
    }

    //////////////////////////////////
    // Connection management
    //////////////////////////////////

    std::shared_ptr<AbstractConnection> ConnectionManager::GetConnection(const std::string& hostname)
    {
      std::lock_guard<decltype(connection_list_mutex_)> connection_list_lock(connection_list_mutex_);
      
      auto connection_it = connections_.find(hostname);
      if (connection_it == connections_.end())
      {
        std::shared_ptr<AbstractConnection> new_connection;
        if (hostname == eCAL::Process::GetHostName())
        {
          new_connection = std::make_shared<LocalConnection>();
        }
        else
        {
          new_connection = std::make_shared<RemoteConnection>(hostname);
        }
        connections_.emplace(hostname, new_connection);
        return new_connection;
      }
      else
      {
        return connection_it->second;
      }
    }
  }
}