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

#include "task_list_thread.h"

#include <chrono>
#include <list>

#include "ecalsys/task/ecal_sys_task.h"

class StartTaskListThread :
  public TaskListThread
{
public:
  StartTaskListThread(const std::list<std::shared_ptr<EcalSysTask>>& task_list, const std::shared_ptr<eCAL::sys::ConnectionManager>& connection_manager, const std::string& target_override = "");

  // Copy construction is not allowed for threads
  StartTaskListThread(StartTaskListThread const&) = delete;
  StartTaskListThread& operator=(StartTaskListThread const&) = delete;

  ~StartTaskListThread();

protected:
  void Run();

private:
  std::string                             m_target_override;                    /**< When not empty, the task will be started on that given host.Otherwise, the configured target is used. */
};

