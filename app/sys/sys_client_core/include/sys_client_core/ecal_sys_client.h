/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

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