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

class UpdateFromCloudTaskListThread :
  public TaskListThread
{
public:
  UpdateFromCloudTaskListThread(const std::list<std::shared_ptr<EcalSysTask>>& task_list_to_update, const std::list<std::shared_ptr<EcalSysTask>>& all_tasks, const std::shared_ptr<eCAL::sys::ConnectionManager>& connection_manager, bool use_localhost_for_all_tasks);

  // Copy construction is not allowed for threads
  UpdateFromCloudTaskListThread(UpdateFromCloudTaskListThread const&) = delete;
  UpdateFromCloudTaskListThread& operator=(UpdateFromCloudTaskListThread const&) = delete;

  ~UpdateFromCloudTaskListThread();

protected:
  void Run();

private:
  bool                                    m_use_localhost_for_all_tasks;
  std::list<std::shared_ptr<EcalSysTask>> m_all_tasks;                          /**< List of all tasks, even those that we don't update. We need this to not double-assign PIDs */
};

