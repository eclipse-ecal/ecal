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

/**
 * eCALSys Util
**/

#include "ecalsys_util.h"
#include <iostream>

#ifdef ECAL_OS_WINDOWS
#include <conio.h>
#else
#include <stdio.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int _kbhit()
{
  static const int STDIN = 0;
  static bool initialized = false;

  if (!initialized)
  {
    // Use termios to turn off line buffering
    termios term;
    tcgetattr(STDIN, &term);
    term.c_lflag &= ~ICANON;
    tcsetattr(STDIN, TCSANOW, &term);
    setbuf(stdin, NULL);
    initialized = true;
  }

  int bytesWaiting;
  ioctl(STDIN, FIONREAD, &bytesWaiting);
  return bytesWaiting;
}

int _getch(void)
{
  struct termios oldattr, newattr;
  int ch;
  tcgetattr(STDIN_FILENO, &oldattr);
  newattr = oldattr;
  newattr.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
  return ch;
}
#endif  // ECAL_OS_WINDOWS

void StartTasks(std::shared_ptr<EcalSys> ecalsys_inst)
{
  std::list<std::shared_ptr<EcalSysTask>> task_list_all = ecalsys_inst->GetTaskList();
  std::list<std::shared_ptr<EcalSysTask>> tasks_to_start;

  // If we are only supposed to start local tasks, we need to filter accordingly
  if (ecalsys_inst->GetOptions().local_tasks_only) {
    std::string current_host = eCAL::Process::GetHostName();
    for (auto task : task_list_all) {
      if (Utility::String::icompare(current_host, task->GetTarget()))
      {
        tasks_to_start.push_back(task);
      }
    }
  }
  else
  {
    tasks_to_start = task_list_all;
  }

  ecalsys_inst->StartTaskList(tasks_to_start);
  ecalsys_inst->WaitForTaskActions();
}

void StopTasks(std::shared_ptr<EcalSys> ecalsys_inst)
{
  std::list<std::shared_ptr<EcalSysTask>> task_list_all = ecalsys_inst->GetTaskList();
  std::list<std::shared_ptr<EcalSysTask>> tasks_to_stop;

  // If we are only supposed to stop local tasks, we need to filter accordingly
  if (ecalsys_inst->GetOptions().local_tasks_only)
  {
    std::string current_host = eCAL::Process::GetHostName();
    for (auto task : task_list_all)
    {
      std::lock_guard<std::recursive_mutex> task_lock(task->mutex);
      if (Utility::String::icompare(current_host, task->GetTarget()))
      {
        tasks_to_stop.push_back(task);
      }
    }
  }
  else
  {
    tasks_to_stop = task_list_all;
  }

  // When we are monitoring, we know the PID and Host of all eCAL Tasks.
  // Therefore, we want to stop those by Host+PID and only kill the remaining
  // Tasks by name. This hopefully reduces the collateral damage that
  // inevitably occurs when killing all processes that have a specific name.

  std::list<std::shared_ptr<EcalSysTask>> task_list_stop_by_pid;
  std::list<std::shared_ptr<EcalSysTask>> task_list_stop_by_name;

  for (auto& task : tasks_to_stop)
  {
    if ((task->GetHostStartedOn() != "") && (task->GetPids().size() > 0))
    {
      task_list_stop_by_pid.push_back(task);
    }
    else
    {
      task_list_stop_by_name.push_back(task);
    }
  }

  ecalsys_inst->StopTaskList(task_list_stop_by_pid, true, true, false);   // Stop by PID
  ecalsys_inst->StopTaskList(task_list_stop_by_name, true, true, true);   // Stop by name

  // Wait for all Tasks being stopped
  ecalsys_inst->WaitForTaskActions();
}

bool WaitForAllTargetsReachable(std::shared_ptr<EcalSys> ecalsys_inst)
{
  std::set<std::string> targets;
  std::string current_host = eCAL::Process::GetHostName();
  for (auto task : ecalsys_inst->GetTaskList())
  {
    if (task->GetTarget() != current_host)
    {
      targets.emplace(task->GetTarget());
    }
  }

  std::set<std::string> targets_not_reachable = GetTargetsNotReachable(ecalsys_inst, targets);
  std::set<std::string> targets_not_reachable_old;

  while (!targets_not_reachable.empty())
  {
    if (targets_not_reachable != targets_not_reachable_old)
    {
      // print a message with the targets that cannot be reached
      std::cout << std::endl << "The following targets cannot be reached:" << std::endl;
      std::for_each(targets_not_reachable.cbegin(), targets_not_reachable.cend(), [](const std::string& target) { std::cout << target << std::endl; });

      std::cout << std::endl << "Wait to reach all targets... [Press ESC to exit]" << std::endl;
    }

    eCAL::Process::SleepMS(500);
    if (_kbhit() > 0 && _getch() == 0x1B /*ESC*/)
    {
      break;
    }

    targets_not_reachable_old = targets_not_reachable;
    targets_not_reachable = GetTargetsNotReachable(ecalsys_inst, targets);
  }

  return (targets_not_reachable.empty());
}

std::set<std::string> GetTargetsNotReachable(std::shared_ptr<EcalSys> ecalsys_inst, std::set<std::string>& targets)
{
  std::set<std::string> not_reachable;
  for (std::string target : targets)
  {
    if (!ecalsys_inst->IseCALSysClientRunningOnHost(target))
    {
      not_reachable.emplace(target);
    }
  }
  return not_reachable;
}

void showMonitoringSummary(std::shared_ptr<EcalSys> ecalsys_instance)
{
  std::stringstream ss;

  ss << "============================== Monitoring Summary =============================" << std::endl;
  ss << "Tasks:" << std::endl;
  ss << "------" << std::endl;

  for (auto& task : ecalsys_instance->GetTaskList())
  {
    auto task_state = task->GetMonitoringTaskState();
    std::string severity_string, tmp;
    task_state.ToString(severity_string, tmp);
    std::string severity_level_string = (task_state.severity != eCAL_Process_eSeverity::proc_sev_unknown ?
      " Lv " + std::to_string(task_state.severity_level) : "");
    std::string info_string = (task_state.info != "" ? " (" + task_state.info + ")" : "");

    ss << "  " << task->GetName() << " @ " << task->GetHostStartedOn() << ": " << severity_string << severity_level_string << info_string << std::endl;
  }

  ss << "Groups:" << std::endl;
  ss << "-------" << std::endl;

  for (auto group : ecalsys_instance->GetGroupList())
  {
    auto activeState = group->Evaluate();
    if (activeState)
    {
      ss << ("  Group " + group->GetName() + ": " + activeState->GetName()) << std::endl;;
    }
    else
    {
      ss << ("  Group " + group->GetName() + " has no active state") << std::endl;;
    }
  }

  ss << std::endl;

  std::cout << ss.str();
}
