#pragma once

#include "abstract_connection.h"

#include <mutex>

namespace eCAL
{
  namespace sys
  {
    class LocalConnection : public AbstractConnection
    {
    public:
      LocalConnection();
      ~LocalConnection();

      std::vector<int32_t> StartTasks(const std::vector<eCAL::sys_client::StartTaskParameters>& task_list) override;

      std::vector<bool>    StopTasks (const std::vector<eCAL::sys_client::StopTaskParameters>& task_list) override;

      std::vector<std::vector<int32_t>> MatchTasks(const std::vector<eCAL::sys_client::Task>& task_list) override;

    private:
      std::mutex connection_mutex_;
    };
  }
}