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

#include "ecalsys/task/ecal_sys_task.h"

#include <regex>

EcalSysTask::EcalSysTask()
  : m_id                         (0)
  , m_name                       ("")
  , m_target                     ("")
  , m_runner                     (std::shared_ptr<EcalSysRunner>(nullptr))
  , m_algo_path                  ("")
  , m_command_line_arguments     ("")
  , m_working_directory          ("")
  , m_launch_order               (0)
  , m_timeout_after_start        (std::chrono::nanoseconds(0))
  , m_visibility                 (eCAL_Process_eStartMode::proc_smode_normal)

  , m_monitoring_enabled         (true)
  , m_restart_by_severity_enabled(false)
  , m_restart_at_severity        (TaskState())

  , m_config_modified_since_start (false)

  , m_host_started_on            ("")
  , m_pids                       (std::vector<int>())
  , m_start_stop_state           (StartStopState::NotStarted)

  , m_found_in_last_monitor_loop (false)
  , m_found_in_monitor_once      (false)
  , m_task_state                 (TaskState())
{}

EcalSysTask::~EcalSysTask()
{
  std::lock_guard<std::recursive_mutex> lock(mutex);
}

////////////////////////////////////////////////////////////////////////////////
//// Configuration Setters & Getters                                        ////
////////////////////////////////////////////////////////////////////////////////

uint32_t EcalSysTask::GetId()
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  return m_id;
}

std::string EcalSysTask::GetName()
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  return m_name;
}

std::string EcalSysTask::GetTarget()
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  return m_target;
}

std::string EcalSysTask::GetAlgoPath()
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  return m_algo_path;
}

std::string EcalSysTask::GetWorkingDir()
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  return m_working_directory;
}

std::shared_ptr<EcalSysRunner> EcalSysTask::GetRunner()
{
  std::lock_guard<std::recursive_mutex> lock(mutex);
  return m_runner;
}

unsigned int EcalSysTask::GetLaunchOrder()
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  return m_launch_order;
}

std::chrono::nanoseconds EcalSysTask::GetTimeoutAfterStart()
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  return m_timeout_after_start;
}

eCAL_Process_eStartMode EcalSysTask::GetVisibility()
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  return m_visibility;
}

std::string EcalSysTask::GetCommandLineArguments()
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  return m_command_line_arguments;
}

bool EcalSysTask::IsMonitoringEnabled()
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  return m_monitoring_enabled;
}

bool EcalSysTask::IsRestartBySeverityEnabled()
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  return m_restart_by_severity_enabled;
}

TaskState EcalSysTask::GetRestartAtSeverity()
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  return m_restart_at_severity;
}


void EcalSysTask::SetId(uint32_t id)
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  m_id = id;
}

void EcalSysTask::SetName(const std::string& name)
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  m_name = name;
}

void EcalSysTask::SetTarget(const std::string& target)
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  m_target = target;
  m_config_modified_since_start = true;
}

void EcalSysTask::SetAlgoPath(const std::string& algo_path)
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  m_algo_path = algo_path;
  m_config_modified_since_start = true;
}

void EcalSysTask::SetWorkingDir(const std::string& working_dir)
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  m_working_directory = working_dir;
  m_config_modified_since_start = true;
}

void EcalSysTask::SetRunner(std::shared_ptr<EcalSysRunner> runner)
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  m_runner = runner;
  m_config_modified_since_start = true;
}

void EcalSysTask::SetLaunchOrder(unsigned int launch_order)
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  m_launch_order = launch_order;
  m_config_modified_since_start = true;
}

void EcalSysTask::SetTimeoutAfterStart(std::chrono::nanoseconds timeout)
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  m_timeout_after_start = timeout;
  m_config_modified_since_start = true;
}

void EcalSysTask::SetVisibility(eCAL_Process_eStartMode visibility)
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  m_visibility = visibility;
  m_config_modified_since_start = true;
}

void EcalSysTask::SetCommandLineArguments(std::string command_line_args)
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  m_command_line_arguments = command_line_args;
  m_config_modified_since_start = true;
}

void EcalSysTask::SetMonitoringEnabled(bool enabled)
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  m_monitoring_enabled = enabled;
  m_config_modified_since_start = true;
}

void EcalSysTask::SetRestartBySeverityEnabled(bool enabled)
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  m_restart_by_severity_enabled = enabled;
  m_config_modified_since_start = true;
}

void EcalSysTask::SetRestartAtSeverity(const TaskState& severity)
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  m_restart_at_severity = severity;
  m_config_modified_since_start = true;
}


bool EcalSysTask::IsConfigModifiedSinceStart()
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  return IsProcessRunning() && m_config_modified_since_start;
}

void EcalSysTask::ResetConfigModifiedSinceStart()
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  m_config_modified_since_start = false;
}

////////////////////////////////////////////////////////////////////////////////
//// Information on the start state                                         ////
////////////////////////////////////////////////////////////////////////////////

std::string EcalSysTask::GetHostStartedOn()
{
  std::lock_guard<std::recursive_mutex> lock(mutex);
  return m_host_started_on;
}

void EcalSysTask::SetHostStartedOn(const std::string& host_name)
{
  std::lock_guard<std::recursive_mutex> lock(mutex);
  m_host_started_on = host_name;
}


std::vector<int> EcalSysTask::GetPids()
{
  std::lock_guard<std::recursive_mutex> lock(mutex);
  return m_pids;
}


void EcalSysTask::SetPids(const std::vector<int>& pid_list)
{
  std::lock_guard<std::recursive_mutex> lock(mutex);
  m_pids = pid_list;
}


EcalSysTask::StartStopState EcalSysTask::GetStartStopState()
{
  std::lock_guard<std::recursive_mutex> lock(mutex);
  return m_start_stop_state;
}


void EcalSysTask::SetStartStopState(const StartStopState start_stop_state)
{
  std::lock_guard<std::recursive_mutex> lock(mutex);
  m_start_stop_state = start_stop_state;
}

////////////////////////////////////////////////////////////////////////////////
//// Process Monitoring                                                     ////
////////////////////////////////////////////////////////////////////////////////

void EcalSysTask::SetMonitoringTaskState(const TaskState& task_state)
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  m_task_state = task_state;
}

TaskState EcalSysTask::GetMonitoringTaskState()
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);
  return m_task_state;
}

bool EcalSysTask::FoundInLastMonitorLoop() 
{
  return m_found_in_last_monitor_loop;
}

void EcalSysTask::SetFoundInLastMonitorLoop(bool found_in_monitoring)
{
  m_found_in_last_monitor_loop = found_in_monitoring;
}

bool EcalSysTask::FoundInMonitorOnce()
{
  return m_found_in_monitor_once;
}

void EcalSysTask::SetFoundInMonitorOnce(bool found_in_monitor_once)
{
  m_found_in_monitor_once = found_in_monitor_once;
}

bool EcalSysTask::IsProcessRunning()
{
  std::lock_guard<std::recursive_mutex> task_lock(mutex);

  if (!(GetStartStopState() == StartStopState::Started_Successfully))
  {
    // How should the process run, if it hasn't been started successfully?
    return false;
  }
  else
  {
    if (FoundInMonitorOnce() && !FoundInLastMonitorLoop())
    {
      // If the task is an eCAL Task, it would be found in monitoring it it was running!
      return false;
    }

    // If the Process has been started successfully and the monitor does not tell us otherwise, we can assume it is still running.
    return true;
  }
}