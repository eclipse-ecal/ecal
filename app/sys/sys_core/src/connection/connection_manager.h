#pragma once

#include <memory>
#include <mutex>

#include "abstract_connection.h"
#include "local_connection.h"
#include "remote_connection.h"

namespace eCAL
{
  namespace sys
  {
    class ConnectionManager
    {
    //////////////////////////////////
    // Constructor & Destructor
    //////////////////////////////////
    public:
      ConnectionManager();
      ~ConnectionManager();

    //////////////////////////////////
    // API
    //////////////////////////////////
    public:
      std::vector<int32_t> StartTasks(const std::string& hostname, const std::vector<eCAL::sys_client::StartTaskParameters>& task_list);

      std::vector<bool>    StopTasks (const std::string& hostname, const std::vector<eCAL::sys_client::StopTaskParameters>& task_list);

      std::vector<std::vector<int32_t>> MatchTasks(const std::string& hostname, const std::vector<eCAL::sys_client::Task>& task_list);


    //////////////////////////////////
    // Connection management
    //////////////////////////////////
    private:
      std::shared_ptr<AbstractConnection> GetConnection(const std::string& hostname);

    //////////////////////////////////
    // Member Variables
    //////////////////////////////////
    private:
      mutable std::mutex connection_list_mutex_;
      std::map<std::string, std::shared_ptr<AbstractConnection>> connections_;
    };
  }
}