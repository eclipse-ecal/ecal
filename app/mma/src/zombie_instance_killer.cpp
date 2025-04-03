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

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <list>
#include <string>

#ifdef _WIN32
#include <windows.h>
#include <algorithm>
#include <direct.h>
#include <TlHelp32.h>
#endif

#ifdef __linux__
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "ecal/process.h"
#include "../include/zombie_instance_killer.h"

bool ZombieInstanceKiller::KillZombieInstance(const std::list<std::string>& process_names)
{
  bool ret_val = false;
  if (!process_names.empty())
  {
    for (const auto& pid_name : process_names)
    {
      ret_val = KillZombie(pid_name);
    }
  }

  return ret_val;
}


#ifdef _WIN32
bool ZombieInstanceKiller::KillZombie(const std::string& process_name)
{
  bool ret_state = false;
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnapshot)
  {
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnapshot, &pe32))
    {
      do
      {
#ifdef _UNICODE
        std::wstring pname     = pe32.szExeFile;
        std::wstring wpid_name = String2WString(process_name);
        DWORD        pid       = pe32.th32ProcessID;
        // cause warning C4244 with VS2017
        //std::transform(pname.begin(), pname.end(), pname.begin(), ::tolower);
        std::transform(pname.begin(), pname.end(), pname.begin(),
          [](char c) {return static_cast<char>(::tolower(c)); });
        if (pname == wpid_name)
#else /* _UNICODE */
        std::string  pname = pe32.szExeFile;
        DWORD        pid   = pe32.th32ProcessID;
        // cause warning C4244 with VS2017
        //std::transform(pname.begin(), pname.end(), pname.begin(), ::tolower);
        std::transform(pname.begin(), pname.end(), pname.begin(),
          [](char c) {return static_cast<char>(::tolower(c)); });
        if (pname == process_name)
#endif /* _UNICODE */
        {
          if (pid != GetCurrentProcessId())
          {
            eCAL::Process::StopProcess(pid);
            ret_state = true;
          }
        }
      } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
  }
  return(ret_state);
}

std::wstring ZombieInstanceKiller::String2WString(const std::string& s)
{
  int len;
  int slength = (int)s.length() + 1;
  len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
  wchar_t* buf = new wchar_t[len];
  MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
  std::wstring r(buf);
  delete[] buf;
  return r;
}

#elif defined(__linux__)
bool ZombieInstanceKiller::KillZombie(const std::string& pid_name)
{
  bool return_value = false;

  std::string command = "pidof " + pid_name;
  FILE* pipe = popen(command.c_str(), "r");

  if (pipe != nullptr)
  {
    char buf[512];
    if (fgets(buf, 512, pipe) == nullptr) return false;

    std::list<std::string> process_ids;
    char* pch;
    pch = strtok(buf, " ");
    while (pch != nullptr)
    {
      process_ids.push_back(pch);
      pch = strtok(nullptr, " ");
    }

    if (!process_ids.empty())
    {
      for (const auto& process_id : process_ids)
      {
        pid_t current_pid = strtoul(process_id.c_str(), nullptr, 10);

        if ((current_pid != 0) && (current_pid != getpid()))
        {
          std::string result_str = eCAL::Process::StopProcess(current_pid) ? "SUCCESS: " : "FAIL: ";

          std::cout << result_str << "Terminating the process with PID : " << current_pid << std::endl;

          return_value = true;
        }
      }
    }
    pclose(pipe);
  }

  return return_value;
}

#else
bool ZombieInstanceKiller::KillZombie(const std::string& pid_name)
{
  return false;
}
#endif
