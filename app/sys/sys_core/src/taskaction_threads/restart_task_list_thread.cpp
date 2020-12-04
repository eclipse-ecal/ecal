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

#include "restart_task_list_thread.h"

#include <map>

RestartTaskListThread::RestartTaskListThread(const std::list<std::shared_ptr<EcalSysTask>>& task_list, const std::shared_ptr<eCAL::sys::ConnectionManager>& connection_manager, bool request_shutdown, bool kill_process, const std::string& target_override, bool by_name, std::chrono::nanoseconds wait_for_shutdown)
  : TaskListThread     (task_list, connection_manager)
  , m_request_shutdown (request_shutdown)
  , m_kill_process     (kill_process)
  , m_target_override  (target_override)
  , m_by_name          (by_name)
  , m_wait_for_shutdown(wait_for_shutdown)
{}

RestartTaskListThread::~RestartTaskListThread()
{}

void RestartTaskListThread::Run()
{
  {
    std::lock_guard<std::mutex> thread_lock(m_thread_mutex);
    if (IsInterrupted()) {
      return;
    }
    m_stop_task_list_thread = std::unique_ptr<StopTaskListThread>(new StopTaskListThread(m_task_list, m_connection_manager, m_request_shutdown, m_kill_process, m_by_name, m_wait_for_shutdown));
    m_stop_task_list_thread->Start();
  }
  m_stop_task_list_thread->Join();

  if (IsInterrupted()) return;

  {
    std::lock_guard<std::mutex> thread_lock(m_thread_mutex);
    if (IsInterrupted()) {
      return; 
    }
    m_start_task_list_thread = std::unique_ptr<StartTaskListThread>(new StartTaskListThread(m_task_list, m_connection_manager, m_target_override));
    m_start_task_list_thread->Start();
  }
  m_start_task_list_thread->Join();
}

void RestartTaskListThread::Interrupt()
{
  InterruptibleThread::Interrupt();
  std::lock_guard<std::mutex> thread_lock(m_thread_mutex);
  if (m_start_task_list_thread)
  {
    m_start_task_list_thread->Interrupt();
  }
  if (m_stop_task_list_thread)
  {
    m_stop_task_list_thread->Interrupt();
  }
}