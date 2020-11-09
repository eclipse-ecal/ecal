#pragma once

#include <vector>

#include "task.h"

namespace eCAL
{
  namespace sys_client
  {
    std::vector<int32_t> StartTasks(const std::vector<StartTaskParameters>& task_list);

    std::vector<bool>    StopTasks (const std::vector<StopTaskParameters>& task_list);

    std::vector<std::vector<int32_t>> MatchTasks(const std::vector<Task>& task_list);
  }
}