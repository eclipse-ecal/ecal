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

#include "start_task_list_thread.h"
#include "stop_task_list_thread.h"

class RestartTaskListThread :
  public TaskListThread
{
public:
  RestartTaskListThread(const std::list<std::shared_ptr<EcalSysTask>>& task_list, const std::shared_ptr<eCAL::sys::ConnectionManager>& connection_manager, bool request_shutdown, bool kill_process, const std::string& target_override = "", bool by_name = false, std::chrono::nanoseconds wait_for_shutdown = std::chrono::seconds(3));

  // Copy construction is not allowed for threads
  RestartTaskListThread(RestartTaskListThread const&) = delete;
  RestartTaskListThread& operator=(RestartTaskListThread const&) = delete;

  ~RestartTaskListThread();

  // Override Interrupt to relay it to the restarting tasks
  void Interrupt();

protected:
  void Run();

private:
  // Data from the constructor
  bool                     m_request_shutdown;                                  /**< Whether a eCAL shutdown request shall be sent to the task */
  bool                     m_kill_process;                                      /**< Whether the task shall be hard-killed */
  std::string              m_target_override;                                   /**< When not empty, the task will be started on that given host. Otherwise, the configured target is used. */
  bool                     m_by_name;                                           /**< Whether the task shall be killed by it's name rather than the known PID (only needed when killing non-eCAL Task from the command line where their PID is unknown */
  std::chrono::nanoseconds m_wait_for_shutdown;                                 /**< Time to wait for a gracefull shutdown, if both a shutdown request shall be sent and the task shall be killed afterwards */

  std::unique_ptr<StopTaskListThread>  m_stop_task_list_thread;                 /**< The thread that is actually stopping the tasks */
  std::unique_ptr<StartTaskListThread> m_start_task_list_thread;                /**< The thread that is actually starting the tasks */
  std::mutex                           m_thread_mutex;                          /**< Mutex for protecting the two threads */
};

