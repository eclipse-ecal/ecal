/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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

#include "ecal_sys_monitor.h"

#include <chrono>
#include <algorithm>

#include <ecal/ecal.h>

#include "ecal/pb/monitoring.pb.h"
#include "ecal/pb/sys/state.pb.h"
#include "ecal/pb/process.pb.h"

#include "ecalsys/ecal_sys_logger.h"

#include <ecal_utils/string.h>
#include <ecal_utils/ecal_utils.h>
#include <ecal_utils/filesystem.h>



EcalSysMonitor::EcalSysMonitor(EcalSys& ecalsys_instance, std::chrono::nanoseconds loop_time)
  : InterruptibleLoopThread(loop_time)
  , m_ecalsys_instance(ecalsys_instance)
  , m_state_publisher("__ecalsys_state__")
  , m_all_hosts()
  , m_hosts_running_ecal_sys_client()
  , m_hosts_running_ecalsys()
  , m_monitor_update_callback_valid(false)
{
  m_monitoring_pb.Clear();
}


EcalSysMonitor::~EcalSysMonitor()
{}

void EcalSysMonitor::Loop()
{
  UpdateMonitor();
  if (IsInterrupted()) {
    return;
  }
  UpdateTaskStates(m_task_list);
  if (IsInterrupted()) {
    return;
  }
  RestartBySeverity();
  if (IsInterrupted()) {
    return;
  }
  {
    std::lock_guard<std::mutex> callback_lock(m_monitor_update_callbacks_mutex);
    if (m_monitor_update_callback_valid)
    {
      m_monitor_update_callback();
    }
  }

  SendEcalsysState();
}

void EcalSysMonitor::UpdateMonitor()
{
  std::string monitoring_string;
  if (eCAL::Monitoring::GetMonitoring(monitoring_string))
  {
    std::lock_guard<std::recursive_mutex> lock(m_monitoring_mutex);
    m_monitoring_pb.Clear();
    m_monitoring_pb.ParseFromString(monitoring_string);

    // Clear all lists
    m_all_hosts.clear();
    m_hosts_running_ecal_sys_client.clear();
    m_hosts_running_ecalsys.clear();

    for (const auto& process : m_monitoring_pb.processes())
    {
      // Update list of all Hosts
      m_all_hosts.emplace(process.hname());

      //Update list of available Targets
      if (process.uname() == "eCALSysClient")
      {
        m_hosts_running_ecal_sys_client.emplace(process.hname());
      }
      // Update list of hosts running eCAL Sys
      if ((process.uname() == "eCALSys") || (process.uname() == "eCALSysGUI"))
      {
        m_hosts_running_ecalsys.push_back(std::pair<std::string, int>(process.hname(), process.pid()));
      }
    }
  }
  else
  {
    EcalSysLogger::Log("eCAL::Monitoring::GetMonitoring - failure", spdlog::level::debug);
  }
  m_task_list = m_ecalsys_instance.GetTaskList();
}

void EcalSysMonitor::UpdateTaskStates(const std::list<std::shared_ptr<EcalSysTask>>& task_list)
{
  for (auto task : task_list)
  {
    if (IsInterrupted())
    {
      return;
    }

    bool is_starting_or_stopping = m_ecalsys_instance.IsTaskActionRunning(task);

    std::lock_guard<std::recursive_mutex> task_lock(task->mutex);

    // This function is publicly callable. Thus we have to lock the monitoring mutex again, as the caller might not live in this thread!
    std::lock_guard<std::recursive_mutex> lock(m_monitoring_mutex);

    TaskState task_state;
    task_state.severity = eCAL_Process_eSeverity::proc_sev_unknown;
    task_state.severity_level = eCAL_Process_eSeverity_Level::proc_sev_level1;
    bool task_mapping_found = false;

    if (!(task->IsMonitoringEnabled()))
    {
      // No monitoring => leave the default severity
      task->SetFoundInMonitorOnce(false);
    }
    else {
      // Monitoring enabled => search in monitored processes for the current task
      for (auto& process : m_monitoring_pb.processes())
      {
        std::vector<int> task_pids = task->GetPids();


        if ((task->GetHostStartedOn() == process.hname())
          && (std::find(task_pids.begin(), task_pids.end(), (int)process.pid()) != task_pids.end()))
        {
          // The task is matching!
          task_mapping_found = true;
          task_state = ConvertState(process.state());
          break;
        }
      }
    }

    // Set whether the task has been found
    if (task_mapping_found)
    {
      task->SetFoundInMonitorOnce(true);
    }
    task->SetFoundInLastMonitorLoop(task_mapping_found);

    // add information to the task_state, if we didn't get any information from the monitoring
    if (!task_mapping_found)
    {
      auto task_start_stop_state = task->GetStartStopState();
      if (task_start_stop_state == EcalSysTask::StartStopState::Started_Successfully)
      {
        if (task->IsMonitoringEnabled() && task->FoundInMonitorOnce() && !is_starting_or_stopping)
        {
          task_state.info = "Externally closed";
          task_state.severity = eCAL_Process_eSeverity::proc_sev_failed;
          task_state.severity_level = eCAL_Process_eSeverity_Level::proc_sev_level5;
        }
        else
        {
          task_state.info = "Started successfully";
        }
      }
      else if (task_start_stop_state == EcalSysTask::StartStopState::Started_Failed)
      {
        task_state.info = "Start failed";
        task_state.severity = eCAL_Process_eSeverity::proc_sev_failed;
        task_state.severity_level = eCAL_Process_eSeverity_Level::proc_sev_level5;
      }
      else if (task_start_stop_state == EcalSysTask::StartStopState::Stopped)
      {
        task_state.info = "Stopped";
      }
    }

    task->SetMonitoringTaskState(task_state);
  }
}

void EcalSysMonitor::RestartBySeverity()
{
  std::list<std::shared_ptr<EcalSysTask>> tasks_for_restarting;

  for (auto& task : m_task_list)
  {
    bool is_starting_or_stopping = m_ecalsys_instance.IsTaskActionRunning(task);

    std::lock_guard<std::recursive_mutex> task_lock(task->mutex);
    // tasks on other hosts than local won't be restarted if the flag local_tasks_only is set
    if ((m_ecalsys_instance.GetOptions().local_tasks_only == true) && (task->GetTarget() != eCAL::Process::GetHostName()))
    {
      continue;
    }

    if (task->IsMonitoringEnabled()
      && task->IsRestartBySeverityEnabled()
      && (task->GetStartStopState() == EcalSysTask::StartStopState::Started_Successfully
        || task->GetStartStopState() == EcalSysTask::StartStopState::Started_Failed))
    {
      TaskState current_state = task->GetMonitoringTaskState();
      TaskState restart_state = task->GetRestartAtSeverity();

      if (current_state.severity != eCAL_Process_eSeverity::proc_sev_unknown
        && current_state >= restart_state)
      {
        if (!is_starting_or_stopping)
        {
          std::string severity_string;
          std::string severity_level_string;
          current_state.ToString(severity_string, severity_level_string);
          EcalSysLogger::Log("Restarting \"" + task->GetName() + "\" due to its Severity (" + severity_string + ", " + severity_level_string + ")");
          tasks_for_restarting.push_back(task);
        }
      }
    }
  }

  if (tasks_for_restarting.size() != 0)
  {
    m_ecalsys_instance.RestartTaskList(tasks_for_restarting, false, true);
  }
}

std::set<std::string> EcalSysMonitor::GetAllHosts()
{
  std::lock_guard<std::recursive_mutex> lock(m_monitoring_mutex);
  return m_all_hosts;
}

std::set<std::string> EcalSysMonitor::GetHostsRunningeCALSysClient()
{
  std::lock_guard<std::recursive_mutex> lock(m_monitoring_mutex);
  return m_hosts_running_ecal_sys_client;
}

std::vector<std::pair<std::string, int>> EcalSysMonitor::GetHostsRunningEcalSys()
{
  std::lock_guard<std::recursive_mutex> lock(m_monitoring_mutex);
  return m_hosts_running_ecalsys;
}

////////////////////////////////////////////////////////////////////////////////
//// Update From cloud                                                      ////
////////////////////////////////////////////////////////////////////////////////

std::list<std::shared_ptr<EcalSysTask>> EcalSysMonitor::GetTasksFromCloud()
{
  std::list<std::shared_ptr<EcalSysTask>> task_list;

  std::lock_guard<std::recursive_mutex> monitoring_lock(m_monitoring_mutex);

  for (const auto& monitor_process : m_monitoring_pb.processes())
  {
    std::string monitor_process_name = EcalUtils::String::Trim(monitor_process.uname());
    std::string monitor_process_path = EcalUtils::String::Trim(monitor_process.pname());
    std::string monitor_process_args = EcalUtils::String::Trim(monitor_process.pparam());
    std::string monitor_process_host = EcalUtils::String::Trim(monitor_process.hname());
    int pid                          = monitor_process.pid();
    TaskState task_state             = ConvertState(monitor_process.state());

    // If the process has no name we use the executable's name instead
    if (monitor_process_name == "")
    {
      monitor_process_name = EcalUtils::Filesystem::BaseName(monitor_process_path);
    }

    // The first argument is always the process itself. Thus, we remove the first argument.
    monitor_process_args = RemoveFirstArg(monitor_process_args);

    std::string algo_path;
    std::string algo_params;

    std::vector<std::string> algo_cmdline_vector = EcalUtils::CommandLine::splitCommandLine(monitor_process.pparam(), 2); // Split command line in algo + arguments
    if(algo_cmdline_vector.size() == 0)
    {
      algo_path = monitor_process_path;
    }
    else if (algo_cmdline_vector.size() == 1)
    {
      algo_path = algo_cmdline_vector[0];
    }
    else
    {
      algo_path   = algo_cmdline_vector[0];
      algo_params = algo_cmdline_vector[1];
    }

    // Create a new ecalsys task with that data
    std::shared_ptr<EcalSysTask> task(new EcalSysTask());

    task->SetName                  (monitor_process_name);
    task->SetTarget                (monitor_process_host);
    task->SetAlgoPath              (algo_path);
    task->SetCommandLineArguments  (algo_params);
    task->SetMonitoringEnabled     (true);
    task->SetHostStartedOn         (monitor_process_host);
    task->SetPids                  (std::vector<int>{pid});
    task->SetStartStopState        (EcalSysTask::StartStopState::Started_Successfully);
    task->SetFoundInLastMonitorLoop(true);
    task->SetFoundInMonitorOnce    (true);
    task->SetMonitoringTaskState   (task_state);

    task_list.push_back(task);
  }

  return task_list;
}

void EcalSysMonitor::SendEcalsysState()
{
  eCAL::pb::sys::State ecalsys_state_pb;

  ecalsys_state_pb.set_host(eCAL::Process::GetHostName());

  // Fill the task field
  auto task_list = m_ecalsys_instance.GetTaskList();
  for (auto& task : task_list)
  {
    eCAL::pb::sys::State::Task* task_pb = ecalsys_state_pb.add_tasks();
    task_pb->CopyFrom(ConvertTask(task));
  }

  // Fill the functions field
  auto group_list = m_ecalsys_instance.GetGroupList();
  for (auto& group : group_list)
  {
    eCAL::pb::sys::State::Function* group_pb = ecalsys_state_pb.add_functions();

    group_pb->set_name(group->GetName());

    // Set the group state
    auto current_group_state = group->Evaluate();
    if (current_group_state)
    {
      group_pb->set_state(current_group_state->GetName());

      eCAL::pb::sys::State::Colour colour_pb;
      colour_pb.set_r(current_group_state->GetColor().red);
      colour_pb.set_g(current_group_state->GetColor().green);
      colour_pb.set_b(current_group_state->GetColor().blue);

      group_pb->mutable_colour()->CopyFrom(colour_pb);
    }

    // Set the list of all tasks
    for (auto& task : group->GetAllTasks())
    {
      eCAL::pb::sys::State::Task* task_pb = group_pb->add_tasks();
      task_pb->CopyFrom(ConvertTask(task));
    }
  }

  m_state_publisher.Send(ecalsys_state_pb);
}

////////////////////////////////////////////////////////////////////////////////
//// Callbacks                                                              ////
////////////////////////////////////////////////////////////////////////////////

void EcalSysMonitor::SetMonitorUpdateCallback(const std::function<void(void)>& callback)
{
  std::lock_guard<std::mutex> callback_lock(m_monitor_update_callbacks_mutex);
  m_monitor_update_callback = callback;
  m_monitor_update_callback_valid = true;
}

void EcalSysMonitor::RemoveMonitorUpdateCallback()
{
  std::lock_guard<std::mutex> callback_lock(m_monitor_update_callbacks_mutex);
  m_monitor_update_callback_valid = false;
}



////////////////////////////////////////////////////////////////////////////////
//// Auxiliary functions                                                    ////
////////////////////////////////////////////////////////////////////////////////
std::string EcalSysMonitor::RemoveFirstArg(const std::string& arg_string)
{
  auto arg_list = EcalUtils::CommandLine::splitCommandLine(arg_string, 2);
  return ((arg_list.size() >= 2) ? arg_list[1] : "");
}

TaskState EcalSysMonitor::ConvertState(eCAL::pb::ProcessState pb_process_state)
{
  TaskState state;

  switch (pb_process_state.severity_level())
  {
  case eCAL::pb::eProcessSeverityLevel::proc_sev_level1:
    state.severity_level = eCAL_Process_eSeverity_Level::proc_sev_level1;
    break;
  case eCAL::pb::eProcessSeverityLevel::proc_sev_level2:
    state.severity_level = eCAL_Process_eSeverity_Level::proc_sev_level2;
    break;
  case eCAL::pb::eProcessSeverityLevel::proc_sev_level3:
    state.severity_level = eCAL_Process_eSeverity_Level::proc_sev_level3;
    break;
  case eCAL::pb::eProcessSeverityLevel::proc_sev_level4:
    state.severity_level = eCAL_Process_eSeverity_Level::proc_sev_level4;
    break;
  case eCAL::pb::eProcessSeverityLevel::proc_sev_level5:
    state.severity_level = eCAL_Process_eSeverity_Level::proc_sev_level5;
    break;
  default:
    state.severity_level = eCAL_Process_eSeverity_Level::proc_sev_level1;
  }

  switch (pb_process_state.severity())
  {
  case eCAL::pb::eProcessSeverity::proc_sev_unknown:
    state.severity = eCAL_Process_eSeverity::proc_sev_unknown;
    break;
  case eCAL::pb::eProcessSeverity::proc_sev_healthy:
    state.severity = eCAL_Process_eSeverity::proc_sev_healthy;
    break;
  case eCAL::pb::eProcessSeverity::proc_sev_warning:
    state.severity = eCAL_Process_eSeverity::proc_sev_warning;
    break;
  case eCAL::pb::eProcessSeverity::proc_sev_critical:
    state.severity = eCAL_Process_eSeverity::proc_sev_critical;
    break;
  case eCAL::pb::eProcessSeverity::proc_sev_failed:
    state.severity = eCAL_Process_eSeverity::proc_sev_failed;
    break;
  default:
    state.severity = eCAL_Process_eSeverity::proc_sev_unknown;
  }

  state.info = pb_process_state.info();

  return state;
}

eCAL::pb::ProcessState EcalSysMonitor::ConvertState(TaskState task_state)
{
  eCAL::pb::ProcessState process_state_pb;

  switch (task_state.severity)
  {
  case eCAL_Process_eSeverity::proc_sev_unknown:
    process_state_pb.set_severity(eCAL::pb::eProcessSeverity::proc_sev_unknown);
    break;
  case eCAL_Process_eSeverity::proc_sev_healthy:
    process_state_pb.set_severity(eCAL::pb::eProcessSeverity::proc_sev_healthy);
    break;
  case eCAL_Process_eSeverity::proc_sev_warning:
    process_state_pb.set_severity(eCAL::pb::eProcessSeverity::proc_sev_warning);
    break;
  case eCAL_Process_eSeverity::proc_sev_critical:
    process_state_pb.set_severity(eCAL::pb::eProcessSeverity::proc_sev_critical);
    break;
  case eCAL_Process_eSeverity::proc_sev_failed:
    process_state_pb.set_severity(eCAL::pb::eProcessSeverity::proc_sev_failed);
    break;
  default:
    process_state_pb.set_severity(eCAL::pb::eProcessSeverity::proc_sev_unknown);
    break;
  }

  switch (task_state.severity_level)
  {
  case eCAL_Process_eSeverity_Level::proc_sev_level1:
    process_state_pb.set_severity_level(eCAL::pb::eProcessSeverityLevel::proc_sev_level1);
    break;
  case eCAL_Process_eSeverity_Level::proc_sev_level2:
    process_state_pb.set_severity_level(eCAL::pb::eProcessSeverityLevel::proc_sev_level2);
    break;
  case eCAL_Process_eSeverity_Level::proc_sev_level3:
    process_state_pb.set_severity_level(eCAL::pb::eProcessSeverityLevel::proc_sev_level3);
    break;
  case eCAL_Process_eSeverity_Level::proc_sev_level4:
    process_state_pb.set_severity_level(eCAL::pb::eProcessSeverityLevel::proc_sev_level4);
    break;
  case eCAL_Process_eSeverity_Level::proc_sev_level5:
    process_state_pb.set_severity_level(eCAL::pb::eProcessSeverityLevel::proc_sev_level5);
    break;
  default:
    process_state_pb.set_severity_level(eCAL::pb::eProcessSeverityLevel::proc_sev_level_unknown);
    break;
  }

  process_state_pb.set_info(task_state.info);

  return process_state_pb;
}

eCAL::pb::sys::State::Task EcalSysMonitor::ConvertTask(std::shared_ptr<EcalSysTask> task)
{
  eCAL::pb::sys::State::Task task_pb;
  task_pb.set_id  (task->GetId());
  task_pb.set_name(task->GetName());
  task_pb.set_host(task->GetTarget());
  for (int pid : task->GetPids())
  {
    task_pb.add_pids(pid);
  }

  if (task->FoundInLastMonitorLoop())
  {
    auto task_state_pb = task_pb.mutable_state();
    task_state_pb->CopyFrom(ConvertState(task->GetMonitoringTaskState()));
  }

  return task_pb;
}
