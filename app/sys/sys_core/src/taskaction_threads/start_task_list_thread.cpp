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

#include <ecalsys/task/ecal_sys_task_helper.h>
#include <ecalsys/ecal_sys_logger.h>

#include "start_task_list_thread.h"
#include <map>
#include <future>

StartTaskListThread::StartTaskListThread(const std::list<std::shared_ptr<EcalSysTask>>& task_list, const std::shared_ptr<eCAL::sys::ConnectionManager>& connection_manager, const std::string& target_override)
  : TaskListThread(task_list, connection_manager)
  , m_target_override(target_override)
{}

StartTaskListThread::~StartTaskListThread()
{}

void StartTaskListThread::Run()
{
  std::map<int, std::list<std::shared_ptr<EcalSysTask>>> launch_groups;
  for (auto& task : m_task_list)
  {
    std::lock_guard<std::recursive_mutex> task_lock(task->mutex);
    if (IsInterrupted()) return;

    // collect the tasks that have the same launch order
    launch_groups[(int)task->GetLaunchOrder()].push_back(task);
  }

  if (IsInterrupted()) return;

  // std::map already sorts everything with std::less, which is exactly what we want
  for (auto launch_group_it = launch_groups.begin(); launch_group_it != launch_groups.end(); launch_group_it++)
  //for (auto& task_list : launch_groups)
  {
    // Sort the tasks into different Targets.
    // Also convert the task to the primitive sys_client task struct
    std::map<std::string, std::vector<eCAL::sys_client::StartTaskParameters>> start_tasks_param_map;
    std::map<std::string, std::vector<std::shared_ptr<EcalSysTask>>>          original_task_ptr_map; // Needed for return values

    std::chrono::nanoseconds waiting_time(0);

    for (const auto& task : launch_group_it->second)
    {
      std::lock_guard<std::recursive_mutex> task_lock(task->mutex);
      if (IsInterrupted()) return;

      // Initialize task state
      task->SetStartStopState        (EcalSysTask::StartStopState::NotStarted);
      task->SetFoundInLastMonitorLoop(false);
      task->SetFoundInMonitorOnce    (false);
      task->SetHostStartedOn         ("");
      task->SetPids                  ({});

      TaskState task_state;
      task_state.info           = "";
      task_state.severity       = eCAL::Process::eSeverity::unknown;
      task_state.severity_level = eCAL::Process::eSeverityLevel::level1;
      task->SetMonitoringTaskState(task_state);
      task->ResetConfigModifiedSinceStart();

      // Create primitive StartTaskParameters struct for ecal_sys_client API
      if (m_target_override.empty())
      {
        start_tasks_param_map[task->GetTarget()].push_back(eCAL::sys::task_helpers::ToSysClientStartParameters_NoLock(task));
        original_task_ptr_map[task->GetTarget()].push_back(task);
      }
      else
      {
        start_tasks_param_map[m_target_override].push_back(eCAL::sys::task_helpers::ToSysClientStartParameters_NoLock(task));
        original_task_ptr_map[m_target_override].push_back(task);
      }

      // Check if the task requires us to wait after the start
      waiting_time = std::max(waiting_time, task->GetTimeoutAfterStart());
    }

    // Start all task of this launch group (Hosts are started in parallel)
    std::map<std::string, std::future<std::vector<int32_t>>> future_map;
    for (const auto& host_taskparamlist_pair : start_tasks_param_map)
    {
      future_map.emplace(host_taskparamlist_pair.first, std::async(std::launch::async, [this, &host_taskparamlist_pair]{ return this->m_connection_manager->StartTasks(host_taskparamlist_pair.first, host_taskparamlist_pair.second); }));
    }

    for (const auto& host_taskparamlist_pair : start_tasks_param_map)
    {
      if (IsInterrupted()) return;
      std::vector<int32_t> process_ids = future_map[host_taskparamlist_pair.first].get();
      if (IsInterrupted()) return;

      for (size_t i = 0; (i < process_ids.size()) && (i < original_task_ptr_map[host_taskparamlist_pair.first].size()); i++)
      {
        std::shared_ptr<EcalSysTask> task = original_task_ptr_map[host_taskparamlist_pair.first][i];

        {
          std::lock_guard<std::recursive_mutex> task_lock(task->mutex);
          if (IsInterrupted()) return;

          if (process_ids[i] != 0)
          {
            task->SetPids({process_ids[i]});
            task->SetHostStartedOn(host_taskparamlist_pair.first);
            task->SetStartStopState(EcalSysTask::StartStopState::Started_Successfully);

            EcalSysLogger::Log("Successfully started Task: " + task->GetName() + " @ " + host_taskparamlist_pair.first, spdlog::level::info);
          }
          else
          {
            task->SetPids({});
            task->SetHostStartedOn("");
            task->SetStartStopState(EcalSysTask::StartStopState::Started_Failed);

            EcalSysLogger::Log("FAILED starting Task:      " + task->GetName() + " @ " + host_taskparamlist_pair.first, spdlog::level::err);
          }
        }
      }

      // Log an error for all tasks that we didn't get a response for (e.g. because we weren't able to contact the client)
      for (size_t i = process_ids.size(); i < original_task_ptr_map[host_taskparamlist_pair.first].size(); i++)
      {
        std::shared_ptr<EcalSysTask> task = original_task_ptr_map[host_taskparamlist_pair.first][i];

        task->SetPids({});
        task->SetHostStartedOn("");
        task->SetStartStopState(EcalSysTask::StartStopState::Started_Failed);

        EcalSysLogger::Log("FAILED starting Task:      " + task->GetName() + " @ " + host_taskparamlist_pair.first + " (Unable to contact client)", spdlog::level::err);
      }
    }

    // Wait for the required amount of time before starting the next launch group
    if (std::next(launch_group_it) != launch_groups.end())
    {
      if (IsInterrupted()) return;
      SleepFor(waiting_time);
    }

    if (IsInterrupted()) return;
  }
}
