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

bool WaitForClients(std::shared_ptr<EcalSys> ecalsys_inst)
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
      std::cout << std::endl << "No eCAL Sys Client on the following targets:" << std::endl;
      std::for_each(targets_not_reachable.cbegin(), targets_not_reachable.cend(), [](const std::string& target) { std::cout << target << std::endl; });

      std::cout << std::endl << "Waiting for eCAL Sys Clients... [Press ESC to exit]" << std::endl;
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
