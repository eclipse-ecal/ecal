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

#include "update_from_cloud_task_list_thread.h"

#include <sys_client_core/ecal_sys_client.h>
#include <ecalsys/task/ecal_sys_task_helper.h>
#include <ecalsys/ecal_sys_logger.h>

#include <map>
#include <future>


UpdateFromCloudTaskListThread::UpdateFromCloudTaskListThread(const std::list<std::shared_ptr<EcalSysTask>>& task_list_to_update, const std::list<std::shared_ptr<EcalSysTask>>& all_tasks, const std::shared_ptr<eCAL::sys::ConnectionManager>& connection_manager, bool use_localhost_for_all_tasks)
  : TaskListThread               (task_list_to_update, connection_manager)
  , m_use_localhost_for_all_tasks(use_localhost_for_all_tasks)
  , m_all_tasks                  (all_tasks)
{}

UpdateFromCloudTaskListThread::~UpdateFromCloudTaskListThread()
{}

void UpdateFromCloudTaskListThread::Run()
{
  // Keep track of found / not found tasks for logging
  EcalSysLogger::Log("Updating Tasks from cloud...");
  std::list<std::pair<std::string, std::string>> tasks_found;
  std::list<std::pair<std::string, std::string>> tasks_not_found;

  // Sort the tasks into hosts and create primitive struct representations for the API
  std::map<std::string, std::vector<eCAL::sys_client::Task>>       host_tasklist_map;
  std::map<std::string, std::vector<std::shared_ptr<EcalSysTask>>> host_originaltasklist_map;
  std::map<std::string, std::vector<std::shared_ptr<EcalSysTask>>> host_all_original_tasks_map;

  for (const std::shared_ptr<EcalSysTask>& ecalsys_task : m_task_list)
  {
    std::lock_guard<std::recursive_mutex> task_lock(ecalsys_task->mutex);
    if (IsInterrupted()) return;

    eCAL::sys_client::Task task = eCAL::sys::task_helpers::ToSysClientTask_NoLock(ecalsys_task);

    if (m_use_localhost_for_all_tasks)
    {
      host_tasklist_map        [eCAL::Process::GetHostName()].push_back(task);
      host_originaltasklist_map[eCAL::Process::GetHostName()].push_back(ecalsys_task);
    }
    else
    {
      host_tasklist_map        [ecalsys_task->GetTarget()].push_back(task);
      host_originaltasklist_map[ecalsys_task->GetTarget()].push_back(ecalsys_task);
    }
  }

  for (const std::shared_ptr<EcalSysTask>& ecalsys_task : m_all_tasks)
  {
    if (m_use_localhost_for_all_tasks)
      host_all_original_tasks_map[eCAL::Process::GetHostName()].push_back(ecalsys_task);
    else
      host_all_original_tasks_map[ecalsys_task->GetTarget()].push_back(ecalsys_task);
  }

  // Match the tasks!!!

  // Iterate over all hosts (get a list of all tasks for each host)
  std::map<std::string, std::future<std::vector<std::vector<int32_t>>>> future_map;
  for (const auto& host_tasklist_pair : host_tasklist_map)
  {
    if (IsInterrupted()) return;
    future_map.emplace(host_tasklist_pair.first, std::async(std::launch::async, [this, &host_tasklist_pair]{ return this->m_connection_manager->MatchTasks(host_tasklist_pair.first, host_tasklist_pair.second); }));
  }


  for (const auto& host_tasklist_pair : host_tasklist_map)
  {
    if (IsInterrupted()) return;
    std::vector<std::vector<int32_t>> list_of_pid_lists = future_map[host_tasklist_pair.first].get();
    if (IsInterrupted()) return;

    std::vector<std::shared_ptr<EcalSysTask>>& tasks_on_this_hosts_for_updating = host_originaltasklist_map[host_tasklist_pair.first];
    std::vector<std::shared_ptr<EcalSysTask>>& all_tasks_on_this_hosts          = host_all_original_tasks_map[host_tasklist_pair.first];


    if (!list_of_pid_lists.empty())
    {
      // Iterate over all PID Lists that were returned. We now have a [Task -> Possible PIDs] mapping.
      for (size_t i = 0; (i < list_of_pid_lists.size()) && (i < tasks_on_this_hosts_for_updating.size()); i++)
      {
        std::shared_ptr<EcalSysTask>& ecalsys_task             = tasks_on_this_hosts_for_updating[i];
        std::vector<int32_t>&         possible_pids            = list_of_pid_lists[i];
        bool                          current_task_was_matched = false;

        // Check for each PID, if it has already been assigned to any (other) task on the host
        for (size_t j = 0; j < possible_pids.size(); j++)
        {
          // If the ecalsys_task already has the PID assigned, we consider the task found
          auto pids_of_current_task = ecalsys_task->GetPids();
          if (std::find(pids_of_current_task.begin(), pids_of_current_task.end(), static_cast<int>(possible_pids[j])) != pids_of_current_task.end())
          {
            current_task_was_matched = true;
            break;
          }

          // If not, we check if the PID belongs to any other task
          bool pid_in_use = false;
          for (const std::shared_ptr<EcalSysTask>& task : all_tasks_on_this_hosts)
          {
            auto task_pids = task->GetPids();
            if (std::find(task_pids.begin(), task_pids.end(), static_cast<int>(possible_pids[j])) != task_pids.end())
            {
              pid_in_use = true;
              break;
            }
          }

          // Assign the PID, if currently unused
          if (!pid_in_use)
          {
            ecalsys_task->SetPids({possible_pids[j]});
            ecalsys_task->SetStartStopState(EcalSysTask::StartStopState::Started_Successfully);
            ecalsys_task->SetHostStartedOn(host_tasklist_pair.first);
            ecalsys_task->ResetConfigModifiedSinceStart();

            current_task_was_matched = true;

            break;
          }
        }

        {
          if (current_task_was_matched)
            tasks_found.push_back({ecalsys_task->GetName(), host_tasklist_pair.first});
          else
            tasks_not_found.push_back({ecalsys_task->GetName(), host_tasklist_pair.first});
        }
      }
    }

    // If the pid_list is empty or smaller than expected, we consider everything after that as not found 
    for (size_t i = list_of_pid_lists.size(); i < tasks_on_this_hosts_for_updating.size(); i++)
    {
      tasks_not_found.push_back({tasks_on_this_hosts_for_updating[i]->GetName(), host_tasklist_pair.first});
    }
  }

  if (IsInterrupted()) return;

  // Log which tasks have been found and which have not
  std::stringstream ss;
  ss << std::endl << "The following tasks have been found and updated from cloud:" << std::endl;
  if (tasks_found.size() == 0)
  {
    ss << "  -- no tasks -- " << std::endl;
  }
  for (auto& task_host_pair : tasks_found)
  {
    ss << "  " << task_host_pair.first << " @ " << task_host_pair.second << std::endl;
  }
  ss << std::endl << "The following tasks have NOT been found:" << std::endl;
  if (tasks_not_found.size() == 0)
  {
    ss << "  -- no tasks -- " << std::endl;
  }
  for (auto& task_host_pair : tasks_not_found)
  {
    ss << "  " << task_host_pair.first << " @ " << task_host_pair.second << std::endl;
  }
  ss << std::endl;
  EcalSysLogger::Log(ss.str());
}
