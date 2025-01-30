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

#include "ecal_sys_monitor.h"

#include <chrono>
#include <algorithm>

#include <ecal/ecal.h>

#include <ecal/core/pb/monitoring.pb.h>
#include <ecal/app/pb/sys/state.pb.h>
#include <ecal/core/pb/process.pb.h>

#include "ecalsys/ecal_sys_logger.h"
#include <ecalsys/proto_helpers.h>

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
      m_all_hosts.emplace(process.host_name());

      //Update list of available Targets
      if (process.unit_name() == "eCALSysClient")
      {
        m_hosts_running_ecal_sys_client.emplace(process.host_name());
      }
      // Update list of hosts running eCAL Sys
      if ((process.unit_name() == "eCALSys") || (process.unit_name() == "eCALSysGUI"))
      {
        m_hosts_running_ecalsys.push_back(std::pair<std::string, int>(process.host_name(), process.process_id()));
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
    task_state.severity = eCAL::Process::eSeverity::unknown;
    task_state.severity_level = eCAL::Process::eSeverityLevel::level1;
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


        if ((task->GetHostStartedOn() == process.host_name())
          && (std::find(task_pids.begin(), task_pids.end(), (int)process.process_id()) != task_pids.end()))
        {
          // The task is matching!
          task_mapping_found = true;
          task_state         = eCAL::sys::proto_helpers::FromProtobuf(process.state());
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
          task_state.severity = eCAL::Process::eSeverity::failed;
          task_state.severity_level = eCAL::Process::eSeverityLevel::level5;
        }
        else
        {
          task_state.info = "Started successfully";
        }
      }
      else if (task_start_stop_state == EcalSysTask::StartStopState::Started_Failed)
      {
        task_state.info = "Start failed";
        task_state.severity = eCAL::Process::eSeverity::failed;
        task_state.severity_level = eCAL::Process::eSeverityLevel::level5;
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

      if (current_state.severity != eCAL::Process::eSeverity::unknown
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
    std::string monitor_process_name = EcalUtils::String::Trim(monitor_process.unit_name());
    std::string monitor_process_path = EcalUtils::String::Trim(monitor_process.process_name());
    std::string monitor_process_args = EcalUtils::String::Trim(monitor_process.process_parameter());
    std::string monitor_process_host = EcalUtils::String::Trim(monitor_process.host_name());
    int process_id                          = monitor_process.process_id();
    TaskState task_state             = eCAL::sys::proto_helpers::FromProtobuf(monitor_process.state());

    // If the process has no name we use the executable's name instead
    if (monitor_process_name == "")
    {
      monitor_process_name = EcalUtils::Filesystem::BaseName(monitor_process_path);
    }

    // The first argument is always the process itself. Thus, we remove the first argument.
    monitor_process_args = RemoveFirstArg(monitor_process_args);

    std::string algo_path;
    std::string algo_params;

    std::vector<std::string> algo_cmdline_vector = EcalUtils::CommandLine::splitCommandLine(monitor_process.process_parameter(), 2); // Split command line in algo + arguments
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
    task->SetPids                  (std::vector<int>{process_id});
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
  auto state_pb = eCAL::sys::proto_helpers::ToProtobuf(m_ecalsys_instance);
  m_state_publisher.Send(state_pb);
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
