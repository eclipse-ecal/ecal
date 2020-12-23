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

#include "threading/interruptible_thread.h"

#include <list>

#include "ecalsys/task/ecal_sys_task.h"

#include <connection/connection_manager.h>

/**
 * @brief A thread that operates on a list of tasks. It can be checked, if a certain task is included in that list.
 */
class TaskListThread
  : public InterruptibleThread
{
public:
  /**
   * @brief Creates a new task list thread that will operate on the given task_list
   * @param task_list the list of tasks to operate on
   * @param connection_manager The connection manager responsible for starting, stopping etc. Tasks on different targets
   */
  TaskListThread(const std::list<std::shared_ptr<EcalSysTask>>& task_list, const std::shared_ptr<eCAL::sys::ConnectionManager>& connection_manager)
    : InterruptibleThread()
    , m_task_list(task_list)
    , m_connection_manager(connection_manager)
  {}

  /**
   * @brief Checks if the internal task list contains a specific task
   * @param task The task that shall be checked
   * @return Whether this thread will operate on that task
   */
  bool ContainsTask(const std::shared_ptr<EcalSysTask> task) const
  {
    return std::find(m_task_list.begin(), m_task_list.end(), task) != m_task_list.end();
  }

protected:
  const std::list<std::shared_ptr<EcalSysTask>>         m_task_list;            /**< The list of tasks this thread shall operate on */
  const std::shared_ptr<eCAL::sys::ConnectionManager>   m_connection_manager;   /**< The connection manager for starting and stopping tasks on different hosts */
};
