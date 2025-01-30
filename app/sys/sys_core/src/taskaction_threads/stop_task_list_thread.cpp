/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include "stop_task_list_thread.h"

#include <sys_client_core/ecal_sys_client.h>
#include <ecalsys/task/ecal_sys_task_helper.h>
#include <ecalsys/ecal_sys_logger.h>

#include <future>

#include <map>

StopTaskListThread::StopTaskListThread(const std::list<std::shared_ptr<EcalSysTask>>& task_list, const std::shared_ptr<eCAL::sys::ConnectionManager>& connection_manager, bool request_shutdown, bool kill_process, bool by_name, std::chrono::nanoseconds wait_for_shutdown)
  : TaskListThread     (task_list, connection_manager)
  , m_request_shutdown (request_shutdown)
  , m_kill_process     (kill_process)
  , m_by_name          (by_name)
  , m_wait_for_shutdown(wait_for_shutdown)
{}

StopTaskListThread::~StopTaskListThread()
{}

void StopTaskListThread::Run()
{
  // Sort the tasks into different Targets.
  // Also convert the task to the primitive sys_client task struct
  std::map<std::string, std::vector<eCAL::sys_client::StopTaskParameters>> stop_tasks_param_map;
  std::map<std::string, std::vector<std::shared_ptr<EcalSysTask>>>         original_task_ptr_map; // Needed for return values

  for (const auto& task : m_task_list)
  {
    std::lock_guard<std::recursive_mutex> task_lock(task->mutex);
    if (IsInterrupted()) return;

    // If the host that the task has been started on is unknown we try to stop it on the configured target
    std::string host_to_stop_on = task->GetHostStartedOn();
    if (host_to_stop_on == "")
      host_to_stop_on = task->GetTarget();


    // Create primitive StopTaskParameters struct for ecal_sys_client API
    eCAL::sys_client::StopTaskParameters stop_task_param = eCAL::sys::task_helpers::ToSysClientStopParameters_NoLock(task, m_by_name);

    if (m_by_name)
    {
      stop_tasks_param_map [host_to_stop_on].push_back(std::move(stop_task_param));
      original_task_ptr_map[host_to_stop_on].push_back(task);
    }
    else if (stop_task_param.process_id != 0)
    {
      stop_tasks_param_map [host_to_stop_on].push_back(std::move(stop_task_param));
      original_task_ptr_map[host_to_stop_on].push_back(task);
    }
  }

  if (!stop_tasks_param_map.empty())
  {
    if (IsInterrupted()) return;

    // 1. Stop by eCAL (-> request shutdown)
    if (m_request_shutdown)
    {
      std::map<std::string, std::future<std::vector<bool>>> future_map;

      for (const auto& host_stopparam_pair : stop_tasks_param_map)
      {
        future_map.emplace(host_stopparam_pair.first, std::async(std::launch::async, [this, &host_stopparam_pair]{ return this->m_connection_manager->StopTasks(host_stopparam_pair.first, host_stopparam_pair.second); }));
      }

      for (const auto& host_stopparam_pair : stop_tasks_param_map)
      {
        auto results = future_map[host_stopparam_pair.first].get();

        // Log whether stopping has been successfull
        for (size_t i = 0; (i < results.size()) && (i < host_stopparam_pair.second.size()); i++)
        {
          std::shared_ptr<EcalSysTask> original_task = original_task_ptr_map[host_stopparam_pair.first][i];
          if (results[i])
            EcalSysLogger::Log("Successfully sent shutdown signal: " + original_task->GetName() + " @ " + host_stopparam_pair.first, spdlog::level::info);
          else
            EcalSysLogger::Log("Failed sending shutdown signal:    " + original_task->GetName() + " @ " + host_stopparam_pair.first, spdlog::level::err); // The API should never return false, so this should never happen.
        }

        // Log an error for all tasks that we didn't get a response for (e.g. because we weren't able to contact the client)
        for (size_t i = results.size(); i < host_stopparam_pair.second.size(); i++)
        {
          std::shared_ptr<EcalSysTask> original_task = original_task_ptr_map[host_stopparam_pair.first][i];
          EcalSysLogger::Log("Failed sending shutdown signal:    " + original_task->GetName() + " @ " + host_stopparam_pair.first + " (Unable to contact client)", spdlog::level::err); // The API should never return false, so this should never happen.
        }

        if (IsInterrupted()) return;
      }
    }

    // 2. Wait for some seconds
    if (IsInterrupted()) return;
    if (m_request_shutdown && m_kill_process)
    {
      SleepFor(m_wait_for_shutdown);
      if (IsInterrupted()) return;
    }

    // 3. Kill Processes
    if (m_kill_process)
    {
      std::map<std::string, std::future<std::vector<bool>>> future_map;

      for (auto& host_stopparam_pair : stop_tasks_param_map)
      {
        for (auto& stopparam : host_stopparam_pair.second)
        {
          stopparam.ecal_shutdown = false;
        }

        future_map.emplace(host_stopparam_pair.first, std::async(std::launch::async, [this, &host_stopparam_pair]{ return this->m_connection_manager->StopTasks(host_stopparam_pair.first, host_stopparam_pair.second); }));
      }

      for (auto& host_stopparam_pair : stop_tasks_param_map)
      {
        auto results = future_map[host_stopparam_pair.first].get();

        // Log whether stopping has been successfull
        for (size_t i = 0; (i < results.size()) && (i < host_stopparam_pair.second.size()); i++)
        {
          std::shared_ptr<EcalSysTask> original_task = original_task_ptr_map[host_stopparam_pair.first][i];
          if (results[i])
            EcalSysLogger::Log("Successfully killed task:          " + original_task->GetName() + " @ " + host_stopparam_pair.first, spdlog::level::info);
          else
            EcalSysLogger::Log("FAILED killing task:               " + original_task->GetName() + " @ " + host_stopparam_pair.first, spdlog::level::err); // The API should never return false, so this should never happen.
        }

        // Log an error for all tasks that we didn't get a response for (e.g. because we weren't able to contact the client)
        for (size_t i = results.size(); i < host_stopparam_pair.second.size(); i++)
        {
          std::shared_ptr<EcalSysTask> original_task = original_task_ptr_map[host_stopparam_pair.first][i];
          EcalSysLogger::Log("FAILED killing task:               " + original_task->GetName() + " @ " + host_stopparam_pair.first + " (Unable to contact client)", spdlog::level::err); // The API should never return false, so this should never happen.
        }

        if (IsInterrupted()) return;
      }
    }
  }

  // Assume everything has been stopped
  for (const auto& task : m_task_list)
  {
    std::lock_guard<std::recursive_mutex> task_lock(task->mutex);
    if (IsInterrupted()) return;

    task->SetStartStopState(EcalSysTask::StartStopState::Stopped);
    task->SetPids(std::vector<int>());
    task->SetHostStartedOn("");
  }
}
