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

#include "ecalsys/ecal_sys.h"

#include <iostream>
#include <string>
#include <algorithm>

#include <ecal/ecal.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <ecal/core/pb/ecal.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <ecal_utils/string.h>

#include "ecalsys/esys_defs.h"

#include "config/config_manager.h"

#include "ecalsys/ecal_sys_logger.h"
#include "ecal_sys_monitor.h"

#include "ecalsys/task/ecal_sys_task.h"
#include "ecalsys/task/ecal_sys_runner.h"
#include "ecalsys/task/task_group.h"

#include "connection/connection_manager.h"
#include "taskaction_threads/restart_task_list_thread.h"
#include "taskaction_threads/start_task_list_thread.h"
#include "taskaction_threads/stop_task_list_thread.h"
#include "taskaction_threads/update_from_cloud_task_list_thread.h"

#include <ecalsys/task/ecal_sys_task_helper.h>

////////////////////////////////////////////////////////////////////////////////
//// Constructors                                                           ////
////////////////////////////////////////////////////////////////////////////////

EcalSys::EcalSys()
  : m_is_config_opened(false)
  , m_current_config_path("")
{
  m_options.check_target_reachability = true;
  m_options.kill_all_on_close = false;
  m_options.local_tasks_only = false;
  m_options.use_localhost_for_all_tasks = false;

  LogAppNameVersion();

  m_connection_manager = std::make_shared<eCAL::sys::ConnectionManager>();

  m_monitor_thread = std::unique_ptr<EcalSysMonitor>(new EcalSysMonitor(*this, std::chrono::seconds(1)));
  m_monitor_thread->Start();
}

EcalSys::EcalSys(const std::string& config_path)
  : EcalSys()
{
  m_is_config_opened = LoadConfig(config_path);
}

EcalSys::~EcalSys() 
{
  eCAL::Finalize();

  m_monitor_thread->Interrupt();
  m_monitor_thread->Join();

  InterruptAllTaskActions();
  WaitForTaskActions();
}

////////////////////////////////////////////////////////////////////////////////
//// Config handling                                                        ////
////////////////////////////////////////////////////////////////////////////////

bool EcalSys::LoadConfig(const std::string& path, bool append)
{
  std::lock_guard<std::recursive_mutex> config_lock(m_config_mutex);
  std::lock_guard<std::recursive_mutex> runner_list_lock(m_runner_list_mutex);
  std::lock_guard<std::recursive_mutex> group_list_lock(m_group_list_mutex);
  std::lock_guard<std::recursive_mutex> task_list_lock(m_task_list_mutex);

  bool success = false;
  success = ConfigManager::LoadConfig(*this, path, append);

  if (success)
  {
    if (append)
    {
      EcalSysLogger::Log("Appending config: " + path, spdlog::level::info);
    }
    else
    {
      EcalSysLogger::Log("Using config: " + path, spdlog::level::info);
      m_current_config_path = path;
      m_is_config_opened = true;
    }
  }

  return success;
}

bool EcalSys::SaveConfig(const std::string& path, ConfigVersion version)
{
  std::lock_guard<std::recursive_mutex> config_lock(m_config_mutex);

  bool success = ConfigManager::SaveConfig(*this, path, version);
  if (success)
  {
    m_current_config_path = path;
  }
  return success;
}

void EcalSys::ClearConfig()
{
  std::lock_guard<std::recursive_mutex> config_lock(m_config_mutex);
  std::lock_guard<std::recursive_mutex> runner_list_lock(m_runner_list_mutex);
  std::lock_guard<std::recursive_mutex> group_list_lock(m_group_list_mutex);
  std::lock_guard<std::recursive_mutex> task_list_lock(m_task_list_mutex);

  m_is_config_opened = false;

  m_task_list_action_thread_container.for_each([](const std::shared_ptr<TaskListThread>& thread) { thread->Interrupt(); });
  m_task_list_action_thread_container.for_each([](const std::shared_ptr<TaskListThread>& thread) { thread->Join(); });
  m_task_list_action_thread_container.clear();

  m_runner_list.clear();
  m_task_list.clear();
  m_group_list.clear();

  m_current_config_path = "";
  m_options.check_target_reachability   = true;
  m_options.kill_all_on_close           = false;
  m_options.local_tasks_only            = false;
  m_options.use_localhost_for_all_tasks = false;
}

bool EcalSys::IsConfigOpened()
{
  std::lock_guard<std::recursive_mutex> config_lock(m_config_mutex);
  return bool(m_is_config_opened);
}

std::string EcalSys::GetCurrentConfigPath()
{
  std::lock_guard<std::recursive_mutex> config_lock(m_config_mutex);
  return m_current_config_path;
}

EcalSys::Options EcalSys::GetOptions() {
  std::lock_guard<std::recursive_mutex> config_lock(m_config_mutex);
  return m_options;
}

void EcalSys::SetOptions(const EcalSys::Options& options) {
  std::lock_guard<std::recursive_mutex> config_lock(m_config_mutex);
  m_options = options;
}


////////////////////////////////////////////////////////////////////////////////
//// Initialization                                                         ////
////////////////////////////////////////////////////////////////////////////////

void EcalSys::LogAppNameVersion() const
{
  std::string app_version_header = " " + std::string(ECAL_SYS_LIB_NAME) + " " + std::string(ECAL_SYS_VERSION_STRING) + " ";
  std::string ecal_version_header = " (eCAL Lib " + std::string(ECAL_VERSION) + ") ";

  EcalUtils::String::CenterString(app_version_header, '-', 79);
  EcalUtils::String::CenterString(ecal_version_header, ' ', 79);

  EcalSysLogger::Log("", spdlog::level::info);
  EcalSysLogger::Log(app_version_header, spdlog::level::info);
  EcalSysLogger::Log(ecal_version_header, spdlog::level::info);
  EcalSysLogger::Log("", spdlog::level::info);
}

////////////////////////////////////////////////////////////////////////////////
//// Getting / Removing of Tasks and Runners                                ////
////////////////////////////////////////////////////////////////////////////////

std::list<std::shared_ptr<EcalSysTask>> EcalSys::GetTaskList() const {
  std::lock_guard<std::recursive_mutex> task_list_lock(m_task_list_mutex);
  return std::list<std::shared_ptr<EcalSysTask>>(m_task_list);
}

std::list<std::shared_ptr<EcalSysRunner>> EcalSys::GetRunnerList() const {
  std::lock_guard<std::recursive_mutex> runner_list_lock(m_runner_list_mutex);
  return std::list<std::shared_ptr<EcalSysRunner>>(m_runner_list);
}

std::list<std::shared_ptr<TaskGroup>> EcalSys::GetGroupList() const
{
  std::lock_guard<std::recursive_mutex> group_list_lock(m_group_list_mutex);
  return m_group_list;
}

std::shared_ptr<EcalSysTask> EcalSys::GetTask(uint32_t id) {
  std::lock_guard<std::recursive_mutex> task_list_lock(m_task_list_mutex);
  for (auto& task : m_task_list) {
    if (task->GetId() == id) {
      return task;
    }
  }
  return std::shared_ptr<EcalSysTask>();
}

std::shared_ptr<EcalSysRunner> EcalSys::GetRunner(uint32_t id) {
  std::lock_guard<std::recursive_mutex> runner_list_lock(m_runner_list_mutex);
  for (auto& runner : m_runner_list) {
    if (runner->GetId() == id) {
      return runner;
    }
  }
  return std::shared_ptr<EcalSysRunner>();
}

void EcalSys::AddTask(std::shared_ptr<EcalSysTask> task, bool keep_id)
{
  std::lock_guard<std::recursive_mutex> task_list_lock(m_task_list_mutex);

  if (!keep_id)
  {
    // find the maximum ID
    uint32_t max_id = 0;
    uint32_t new_id = 1;
    for (auto& existing_task : m_task_list)
    {
      // Whoever though that it might be a good idea to clutter ALL namespaces with a #define max should be punished.
#ifdef max
#define max_temp max
#undef max
#endif //max

      max_id = (std::max(max_id, existing_task->GetId()));
      if (max_id != std::numeric_limits<uint32_t>::max())
      {
        // increase the max found ID by 1 to obtain the next free ID
        new_id = max_id + 1;
      }
      else
      {
        // Search for the next free ID from the front
        auto sorted_list = m_task_list;
        sorted_list.sort([](auto t1, auto t2) { return t1->GetId() < t2->GetId(); });

        auto next_task_it = sorted_list.begin();
        while (next_task_it != sorted_list.end())
        {
          if (new_id != (*next_task_it)->GetId())
          {
            // Found a free ID!
            break;
          }
          else
          {
            // Keep looking!
            new_id++;
            next_task_it++;
          }
        }
      }

#ifdef max_temp
#define max max_temp
#undef max_temp
#endif //max_temp
    }

    task->SetId(new_id);
  }

  m_task_list.push_back(task);
}

void EcalSys::AddRunner(std::shared_ptr<EcalSysRunner> runner, bool keep_id)
{
  std::lock_guard<std::recursive_mutex> runner_list_lock(m_runner_list_mutex);

  if (!keep_id)
  {
    // find the maximum ID
    uint32_t max_id = 0;
    uint32_t new_id = 1;
    for (auto& existing_runner : m_runner_list)
    {
      // Whoever though that it might be a good idea to clutter ALL namespaces with a #define max should be punished.
#ifdef max
#define max_temp max
#undef max
#endif //max

      max_id = (std::max(max_id, existing_runner->GetId()));
      if (max_id != std::numeric_limits<uint32_t>::max())
      {
        // increase the max found ID by 1 to obtain the next free ID
        new_id = max_id + 1;
      }
      else
      {
        // Search for the next free ID from the front
        auto sorted_list = m_runner_list;
        sorted_list.sort([](auto t1, auto t2) { return t1->GetId() < t2->GetId(); });

        auto next_runner_it = sorted_list.begin();
        while (next_runner_it != sorted_list.end())
        {
          if (new_id != (*next_runner_it)->GetId())
          {
            // Found a free ID!
            break;
          }
          else
          {
            // Keep looking!
            new_id++;
            next_runner_it++;
          }
        }
      }

#ifdef max_temp
#define max max_temp
#undef max_temp
#endif //max_temp
    }

    runner->SetId(new_id);
  }

  m_runner_list.push_back(runner);
}

void EcalSys::AddTaskGroup(std::shared_ptr<TaskGroup> task_group, bool keep_id)
{
  std::lock_guard<std::recursive_mutex> group_list_lock(m_group_list_mutex);

  if (!keep_id)
  {
    // find the maximum ID
    uint32_t max_id = 0;
    uint32_t new_id = 1;
    for (auto& existing_group : m_group_list)
    {
      // Whoever though that it might be a good idea to clutter ALL namespaces with a #define max should be punished.
#ifdef max
#define max_temp max
#undef max
#endif //max

      max_id = (std::max(max_id, existing_group->GetId()));
      if (max_id != std::numeric_limits<uint32_t>::max())
      {
        // increase the max found ID by 1 to obtain the next free ID
        new_id = max_id + 1;
      }
      else
      {
        // Search for the next free ID from the front
        auto sorted_list = m_group_list;
        sorted_list.sort([](auto t1, auto t2) { return t1->GetId() < t2->GetId(); });

        auto next_group_it = sorted_list.begin();
        while (next_group_it != sorted_list.end())
        {
          if (new_id != (*next_group_it)->GetId())
          {
            // Found a free ID!
            break;
          }
          else
          {
            // Keep looking!
            new_id++;
            next_group_it++;
          }
        }
      }

#ifdef max_temp
#define max max_temp
#undef max_temp
#endif //max_temp
    }

    task_group->SetId(new_id);
  }

  m_group_list.push_back(task_group);
}

std::list<std::shared_ptr<TaskGroup>> EcalSys::RemoveTask(std::shared_ptr<EcalSysTask> task)
{
  std::lock_guard<std::recursive_mutex> group_list_lock(m_group_list_mutex);
  std::lock_guard<std::recursive_mutex> task_list_lock (m_task_list_mutex);

  std::list<std::shared_ptr<TaskGroup>> affected_groups;

  // Remove the task from all groups that contain that task anywhere
  for (auto& group : m_group_list)
  {
    auto state_list = group->GetGroupStateList();
    for (auto& state : state_list)
    {
      auto min_task_state_list = state->GetMinimalStatesList();
      size_t size_before = min_task_state_list.size();
      min_task_state_list.remove_if([&task](auto min_state) {return min_state.first == task; });
      size_t size_after = min_task_state_list.size();

      // check if we had to remove anything
      if (size_before != size_after)
      {
        state->SetMinimalStatesList(min_task_state_list);
        affected_groups.push_back(group);
      }
    }
  }

  m_task_list.remove(task);

  return affected_groups;
}

std::list<std::shared_ptr<EcalSysTask>> EcalSys::RemoveRunner(std::shared_ptr<EcalSysRunner> runner, bool expand_runner_before_removal)
{
  std::lock_guard<std::recursive_mutex> runner_list_lock(m_runner_list_mutex);
  std::lock_guard<std::recursive_mutex> task_list_lock  (m_task_list_mutex);

  std::list<std::shared_ptr<EcalSysTask>> affected_tasks;
  for (auto& task : m_task_list)
  {
    std::lock_guard<std::recursive_mutex> task_lock(task->mutex);
    if (task->GetRunner() && (task->GetRunner() == runner))
    {
      // Collect all tasks that are affected by the runner-removal
      affected_tasks.push_back(task);

      if (expand_runner_before_removal)
      {
        // Convert into a primitive sys_client task
        eCAL::sys_client::Task temp_task = eCAL::sys::task_helpers::ToSysClientTask_NoLock(task);

        // Merge the runner into the task, just as we would start the task.
        // The only differences are:
        //   1. We haven't evaluated the eCAL Parser functions
        //   2. We have no idea of the targets OS
        // 
        // This may lead to some errors, e.g. if an eCAL Parser function is used
        // that prevents the detection whether the algo path is relative or
        // absolut. The user will have to deal with that.

        eCAL::sys_client::MergeRunnerIntoTask(temp_task, EcalUtils::Filesystem::OsStyle::Combined);

        // Copy the values back to the internal task
        task->SetAlgoPath(temp_task.path);
        task->SetCommandLineArguments(temp_task.arguments);
        task->SetWorkingDir(temp_task.working_dir);

        // Remove the runner
        task->SetRunner(std::shared_ptr<EcalSysRunner>());
      }
      else
      {
        // Remove the runner destructively
        task->SetRunner(std::shared_ptr<EcalSysRunner>());
      }
    }
  }

  m_runner_list.remove(runner);

  return affected_tasks;
}

void EcalSys::RemoveTaskGroup(std::shared_ptr<TaskGroup> task_group)
{
  std::lock_guard<std::recursive_mutex> runner_list_lock(m_group_list_mutex);
  m_group_list.remove(task_group);
}


////////////////////////////////////////////////////////////////////////////////
//// Starting / Stopping / Restarting list of tasks                         ////
////////////////////////////////////////////////////////////////////////////////


void EcalSys::StartTaskList(const std::list<std::shared_ptr<EcalSysTask>>& task_list, const std::string& target_override)
{
  std::list<std::shared_ptr<EcalSysTask>> filtered_task_list;
  for (auto& task : task_list)
  {
    // Don't start tasks that are currently starting or stopping
    if (IsTaskActionRunning(task))
      continue;

    // Don't start tasks that are running fine
    if (task->IsProcessRunning())
      continue;

    filtered_task_list.push_back(task);
  }

  std::string actual_target_override = ""; // The options may have an influence on the actual target, as well!
  if (!target_override.empty())
  {
    actual_target_override = target_override;
  }
  else if (GetOptions().use_localhost_for_all_tasks)
  {
    actual_target_override = eCAL::Process::GetHostName();
  }

  std::shared_ptr<TaskListThread> start_task_list_thread(new StartTaskListThread(filtered_task_list, m_connection_manager, actual_target_override));
  m_task_list_action_thread_container.add(start_task_list_thread);
  start_task_list_thread->Start();

  // Remove finished threads (yes, this is an ugly garbage collecting mechanism, which should still pose no problem in this case)
  RemoveFinishedTaskListThreads();
}

void EcalSys::StopTaskList(const std::list<std::shared_ptr<EcalSysTask>>& task_list, bool request_shutdown, bool kill_process, bool by_name, std::chrono::nanoseconds wait_for_shutdown)
{
  std::list<std::shared_ptr<EcalSysTask>> filtered_task_list;
  for (auto& task : task_list)
  {
    if (!IsTaskActionRunning(task))
    {
      filtered_task_list.push_back(task);
    }
  }

  std::shared_ptr<TaskListThread> stop_task_list_thread(new StopTaskListThread(filtered_task_list, m_connection_manager, request_shutdown, kill_process, by_name, wait_for_shutdown));
  m_task_list_action_thread_container.add(stop_task_list_thread);
  stop_task_list_thread->Start();

  // Remove finished threads (yes, this is an ugly garbage collecting mechanism, which should still pose no problem in this case)
  RemoveFinishedTaskListThreads();
}

void EcalSys::RestartTaskList(const std::list<std::shared_ptr<EcalSysTask>>& task_list, bool request_shutdown, bool kill_process, const std::string& target_override, bool by_name, std::chrono::nanoseconds wait_for_shutdown)
{
  std::list<std::shared_ptr<EcalSysTask>> filtered_task_list;
  for (auto& task : task_list)
  {
    if (!IsTaskActionRunning(task))
    {
      filtered_task_list.push_back(task);
    }
  }

  std::string actual_target_override = ""; // The options may have an influence on the actual target, as well!
  if (!target_override.empty())
  {
    actual_target_override = target_override;
  }
  else if (GetOptions().use_localhost_for_all_tasks)
  {
    actual_target_override = eCAL::Process::GetHostName();
  }

  std::shared_ptr<TaskListThread> restart_task_list_thread(new RestartTaskListThread(filtered_task_list, m_connection_manager, request_shutdown, kill_process, actual_target_override, by_name, wait_for_shutdown));
  m_task_list_action_thread_container.add(restart_task_list_thread);
  restart_task_list_thread->Start();

  // Remove finished threads (yes, this is an ugly garbage collecting mechanism, which should still pose no problem in this case)
  RemoveFinishedTaskListThreads();
}

void EcalSys::StartTasks()
{
  auto options = GetOptions();

  if (options.local_tasks_only)
  {
    auto task_list = GetTaskList();
    task_list.remove_if([](const std::shared_ptr<EcalSysTask> t) -> bool { return t->GetTarget() != eCAL::Process::GetHostName(); });
    StartTaskList(task_list);
  }
  else if (options.use_localhost_for_all_tasks)
  {
    StartTaskList(GetTaskList(), eCAL::Process::GetHostName());
  }
  else
  {
    StartTaskList(GetTaskList());
  }
}

void EcalSys::StopTasks()
{
  auto options = GetOptions();

  if (options.local_tasks_only)
  {
    auto task_list = GetTaskList();
    task_list.remove_if([](const std::shared_ptr<EcalSysTask> t) -> bool { return t->GetTarget() != eCAL::Process::GetHostName(); });
    StopTaskList(task_list, true, true);
  }
  else
  {
    StopTaskList(GetTaskList(), true, true);
  }
}

void EcalSys::RestartTasks()
{
  auto options = GetOptions();

  if (options.local_tasks_only)
  {
    auto task_list = GetTaskList();
    task_list.remove_if([](const std::shared_ptr<EcalSysTask> t) -> bool { return t->GetTarget() != eCAL::Process::GetHostName(); });
    RestartTaskList(GetTaskList(), true, true);
  }
  else if (options.use_localhost_for_all_tasks)
  {
    RestartTaskList(GetTaskList(), true, true, eCAL::Process::GetHostName());
  }
  else
  {
    RestartTaskList(GetTaskList(), true, true);
  }}

bool EcalSys::IsTaskActionRunning(const std::shared_ptr<EcalSysTask> task)
{
  bool start_stop_scheduled = false;
  m_task_list_action_thread_container.for_each(
    [&start_stop_scheduled, &task](std::shared_ptr<TaskListThread> t) {start_stop_scheduled = (start_stop_scheduled || (t->IsRunning() && t->ContainsTask(task))); }
  );
  return start_stop_scheduled;
}

void EcalSys::InterruptAllTaskActions()
{
  m_task_list_action_thread_container.for_each(
    [](std::shared_ptr<TaskListThread> t) {t->Interrupt(); }
  );
}

void EcalSys::WaitForTaskActions()
{
  // We must not keep the task_thread container locked while waiting for the
  // threads. Thus we copy the container.

  std::vector<std::shared_ptr<TaskListThread>> copy_of_task_thread_container;
  m_task_list_action_thread_container.for_each(
    [&copy_of_task_thread_container](std::shared_ptr<TaskListThread> t)
    {
      copy_of_task_thread_container.push_back(t);
    }
  );

  for (auto& task_thread : copy_of_task_thread_container)
  {
    task_thread->Join();
  }

  //m_task_list_action_thread_container.for_each(
  //  [](std::shared_ptr<TaskListThread> t)
  //  {
  //    auto process_id = t->get_pid();
  //    std::cout << "Waiting for " << process_id << std::endl;
  //    t->Join(); 
  //    std::cout << "Finished waiting for " << process_id << std::endl;
  //  }
  //);
}

void EcalSys::RemoveFinishedTaskListThreads()
{
  //yes, this is an ugly garbage collecting mechanism, which should still pose no problem in this case
  m_task_list_action_thread_container.remove_if([](std::shared_ptr<TaskListThread> thread) {
    if (!thread->IsRunning())
    {
      thread->Join();
      return true;
    }
    return false;
  });
}


////////////////////////////////////////////////////////////////////////////////
//// Monitoring                                                             ////
////////////////////////////////////////////////////////////////////////////////

bool EcalSys::IseCALSysClientRunningOnHost(const std::string& host_name) const {
  auto hosts_running_ecal_sys_client = m_monitor_thread->GetHostsRunningeCALSysClient();
  for (auto& host : hosts_running_ecal_sys_client)
  {
    if (host == host_name)
    {
      return true;
    }
  }
  return false;
}

void EcalSys::UpdateTaskStates(const std::list<std::shared_ptr<EcalSysTask>>& task_list)
{
  m_monitor_thread->UpdateTaskStates(task_list);
}

void EcalSys::UpdateFromCloud()
{
  auto task_list = GetTaskList();

  std::list<std::shared_ptr<EcalSysTask>> filtered_task_list;
  for (auto& task : task_list)
  {
    // Don't start task actions for tasks that are currently starting or stopping
    if (IsTaskActionRunning(task))
      continue;

    filtered_task_list.push_back(task);
  }

  std::shared_ptr<TaskListThread> update_from_cloud_task_list_thread(new UpdateFromCloudTaskListThread(filtered_task_list, task_list, m_connection_manager, GetOptions().use_localhost_for_all_tasks));
  m_task_list_action_thread_container.add(update_from_cloud_task_list_thread);
  update_from_cloud_task_list_thread->Start();

  // Remove finished threads (yes, this is an ugly garbage collecting mechanism, which should still pose no problem in this case)
  RemoveFinishedTaskListThreads();
}

std::list<std::shared_ptr<EcalSysTask>> EcalSys::GetTasksFromCloud()
{
  return m_monitor_thread->GetTasksFromCloud();
}

void EcalSys::SetMonitorUpdateCallback(const std::function<void(void)>& callback)
{
  m_monitor_thread->SetMonitorUpdateCallback(callback);
}

void EcalSys::RemoveMonitorUpdateCallback()
{
  m_monitor_thread->RemoveMonitorUpdateCallback();
}

std::set<std::string> EcalSys::GetAllHosts()
{
  return m_monitor_thread->GetAllHosts();
}

std::vector<std::pair<std::string, int>> EcalSys::GetHostsRunningEcalSys()
{
  return m_monitor_thread->GetHostsRunningEcalSys();

}

////////////////////////////////////////////////////////////////////////////////
//// Auxiliary                                                              ////
////////////////////////////////////////////////////////////////////////////////
