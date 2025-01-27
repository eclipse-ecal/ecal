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

#include <iostream>

#include "..\..\include\windows\processes.h"

Processes::Processes(void) :
  work_mode(WorkMode_t::DISABLED),
  process_map(),
  process_list(),
  ntdll_module_hdl(nullptr),
  gNtQueryInformationProcess(nullptr)
{
  if (EnableTokenPrivilege() == false)
  {
    Logger::getLogger()->Log("ERROR EnableTokenPrivilege()");
  }

  if (LoadNTdllFunctionality() == false)
  {
    Logger::getLogger()->Log("ERROR LoadNTdllFunctionality()");
    work_mode = REDUCED;
  }
  else
  {
    work_mode = FULLY;
  }
}

Processes::~Processes()
{
  FreeNTdllFunctionality();
}

bool Processes::RefeshData(void)
{
  if (work_mode == DISABLED)
  {
    return false;
  }
  bool return_val = true;
  if (GetListOfProcessIDs() == false)
  {
    Logger::getLogger()->Log("ERROR GetListOfProcessIDs()");
    return_val = false;
  }

  AlignProcessMapWithNewProcessList();
  
  if (UpdateProcessMap() == false)
  {
    Logger::getLogger()->Log("ERROR UpdateProcessMap()");
    return_val = false;
  }

  return return_val;
}

ProcessMap_ptr_t Processes::GetProcessesInfo(void)
{
  if (work_mode != DISABLED)
  {
    return &process_map;
  }
  return nullptr;
}

bool Processes::UpdateProcessMap(void)
{
  bool returnvalue = true;

  for (auto it = process_map.begin(); it != process_map.end(); ++it)
  {
    if (it->second.not_accessible == false)
    {
      // Get a handle to the process.
      HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, it->first);

      if (nullptr != hProcess)
      {
        if ((it->second.is_new == true) && (work_mode == FULLY))
        {
          it->second.is_new = false;
          // Get the process name.
          TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

          if (GetModuleFileNameEx(hProcess, 0, szProcessName, MAX_PATH))
          {
            it->second.name = static_cast<std::string>(szProcessName);
          }
          else
          {
            Logger::getLogger()->Log("ERROR GetModuleFileNameEx(...) : " + std::to_string(GetLastError()));
          }

          static ExtendedProcessInfo_t process_info;

          if (GetExtendedProcessInfo(it->first, &process_info))
          {
            it->second.command_line = static_cast<std::string>(process_info.cmd_line);
            if (process_info.debug_flag == 0)
            {
              it->second.debug_flag = false;
            }
            else
            {
              it->second.debug_flag = true;
            }

          }
          else
          {
            Logger::getLogger()->Log("ERROR GetExtendedProcessInfo(...): " + std::to_string(GetLastError()));
          }
        }

        // Process memory info
        PROCESS_MEMORY_COUNTERS process_memory_counter;
        if (GetProcessMemoryInfo(hProcess, &process_memory_counter, sizeof(process_memory_counter)))
        {
          it->second.memory_peak = static_cast<unsigned long>(process_memory_counter.PeakWorkingSetSize);
          it->second.memory_current = static_cast<unsigned long>(process_memory_counter.WorkingSetSize);
        }
        else
        {
          Logger::getLogger()->Log("ERROR GetProcessMemoryInfo(...): " + std::to_string(GetLastError()));
          it->second.not_accessible = true;
        }

        /* SIZE_T  dwMin, dwMax;
        if (GetProcessWorkingSetSize(hProcess, &dwMin, &dwMax))
        {
        peak_memory_ = dwMax;
        used_memory_ = dwMin;
        }
        else
        {
        std::cout << "ERROR GetProcessWorkingSetSize(): " << GetLastError() << std::endl;
        }*/

        // CPU 
        unsigned long long time_now = GetTickCount() * 10000;   // GetTickCount in ms --> * 10000 to get it in 100-ns units
        if (it->second.last_time_measured != 0)
        {
          FILETIME creation, kernel, user, unused_;
          if (GetProcessTimes(hProcess, &creation, &unused_, &kernel, &user))           //  100-nanosecond time units
          {
            unsigned long long llkernel = ((unsigned long long)kernel.dwHighDateTime << 32) + kernel.dwLowDateTime;
            unsigned long long lluser = ((unsigned long long)user.dwHighDateTime << 32) + user.dwLowDateTime;

            // cpu load in percent and sum (kernel + user) = difference between last and current 

            it->second.cpu_consumption = static_cast<float>((((llkernel - it->second.cpu_time_kernel) + (lluser - it->second.cpu_time_user)) * 10000) / (time_now - it->second.last_time_measured))/100;

            it->second.cpu_time_kernel = llkernel;
            it->second.cpu_time_user = lluser;
            it->second.cpu_creation_time = ((unsigned long long)creation.dwHighDateTime << 32) + creation.dwLowDateTime;
          }
          else
          {
            Logger::getLogger()->Log("ERROR GetProcessTimes(...): " + std::to_string(GetLastError()));
            it->second.not_accessible = true;
          }
        }
        it->second.last_time_measured = time_now;

      }
      else
      {
        it->second.not_accessible = true;
        if (GetLastError() != 5)    // 5 --> ACCESS DENIED
        {
          Logger::getLogger()->Log("ERROR OpenProcess(...): " + std::to_string(GetLastError()));
        }
      }
      CloseHandle(hProcess);
    }
  }
  return returnvalue;
}

void Processes::AlignProcessMapWithNewProcessList()
{
  // 1. Check if there are processes in the map, that are not existing any more
  for (auto it = process_map.cbegin(); it != process_map.cend(); /* no increment */)
  {
    if (std::find(process_list.begin(), process_list.end(), it->first) == process_list.end())
    {
      process_map.erase(it++);
    }
    else
    {
      ++it;
    }
  }

  // 2. Fill up the process map with new process ids
  for (unsigned int index = 0; index < process_list.size(); ++index)
  {
    if (process_map.find(process_list[index]) == process_map.end())
    {
      process_map.insert(ProcessPair_t(process_list[index], ProcessInfo_t()));
    }
  }
}

bool Processes::GetListOfProcessIDs(void)
{
  process_list.assign(1024, 0);
  if (process_list.capacity() != 1024)
  {
    process_list.reserve(1024);
  }
  static DWORD cbNeeded = 0;
  bool return_val = false;


  if (EnumProcesses(process_list.data(), static_cast<DWORD>(process_list.capacity() * sizeof(DWORD)), &cbNeeded))
  {
    // Calculate how many process identifiers were returned.
    return_val = true;
  }
  process_list.resize(cbNeeded / sizeof(DWORD));

  return return_val;
}


bool Processes::EnableTokenPrivilege()
{
  HANDLE hToken = 0;
  TOKEN_PRIVILEGES tkp {};

  // Get a token for this process. 
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
  {
    return false;
  }

  // Get the LUID for the privilege. 
  if (LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &tkp.Privileges[0].Luid))
  {
    tkp.PrivilegeCount = 1;  // one privilege to set    
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // Set the privilege for this process. 
    AdjustTokenPrivileges(hToken, false, &tkp, 0, (PTOKEN_PRIVILEGES)nullptr, 0);

    if (GetLastError() != ERROR_SUCCESS)
    {
      return false;
    }
    return true;
  }
  return false;
}

bool Processes::LoadNTdllFunctionality()
{
  ntdll_module_hdl = LoadLibrary(_T("ntdll.dll"));
  if (ntdll_module_hdl == nullptr)
  {
    return false;
  }

  gNtQueryInformationProcess = (pfnNtQueryInformationProcess)GetProcAddress(ntdll_module_hdl,"NtQueryInformationProcess");
  if (gNtQueryInformationProcess == nullptr) 
  {
    FreeLibrary(ntdll_module_hdl);
    return false;
  }
  return true;
}

void Processes::FreeNTdllFunctionality(void)
{
  if (ntdll_module_hdl)
  {
    FreeLibrary(ntdll_module_hdl);
  }
  gNtQueryInformationProcess = nullptr;
}

bool Processes::GetExtendedProcessInfo(const DWORD _pid, ExtendedProcessInfo_ptr_t extended_process_info_ptr_) //GetExtendedProcessInfo(const DWORD _pid, ExtendedProcessInfo_t *ppi)
{
  bool  bReturnStatus = true;
  
  DWORD dwSizeNeeded = 0;
  SIZE_T dwBytesRead = 0;
  DWORD dwBufferSize = 0;
  HANDLE hHeap = 0;
  WCHAR *pwszBuffer = nullptr;

  ExtendedProcessInfo_t spi {};

  BasicInfoStruct_t peb {};
  ProcessLdrData_t peb_ldr {};
  UserProcessPar_t peb_upp {};

  ZeroMemory(&spi, sizeof(spi));
  ZeroMemory(&peb, sizeof(peb));
  ZeroMemory(&peb_ldr, sizeof(peb_ldr));
  ZeroMemory(&peb_upp, sizeof(peb_upp));

  spi.process_id = _pid;

  HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, _pid);
  if (hProcess == INVALID_HANDLE_VALUE) 
  {
    return false; //-V773
  }

  // Try to allocate buffer 
  hHeap = GetProcessHeap();

  static DWORD dwSize = sizeof(BasisProcessInfo_t);

  BasisProcessInfo_ptr_t pbi = static_cast<BasisProcessInfo_ptr_t>(HeapAlloc(hHeap, HEAP_ZERO_MEMORY,  dwSize));

  if (!pbi) 
  {
    CloseHandle(hProcess);
    return false;
  }

  NTSTATUS dwStatus = gNtQueryInformationProcess(hProcess, ProcessBasicInformation, pbi, dwSize, &dwSizeNeeded);

  if (dwStatus >= 0 && dwSize < dwSizeNeeded)
  {
    HeapFree(hHeap, 0, pbi);
    pbi = static_cast<BasisProcessInfo_ptr_t>(HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwSizeNeeded));
    if (!pbi) 
    {
      CloseHandle(hProcess);
      return false;
    }
    dwStatus = gNtQueryInformationProcess(hProcess, ProcessBasicInformation, pbi, dwSizeNeeded, &dwSizeNeeded);
  }

  if (dwStatus >= 0)
  {
    bool failed_to_free_memory = false;
    // Basic Info
    //        spi._pid         = (DWORD)pbi->UniqueProcessId;
    spi.parent_pid = (DWORD)pbi->InheritedFromUniqueProcessId;
    spi.base_priority = (LONG)pbi->BasePriority;
    spi.exit_status = (NTSTATUS)pbi->ExitStatus;
    //    spi.peb_base_address = (DWORD)pbi->PebBaseAddress;
    spi.affinity_mask = (DWORD)pbi->AffinityMask;

    if (pbi->PebBaseAddress)
    {
      if (ReadProcessMemory(hProcess, pbi->PebBaseAddress, &peb, sizeof(peb), &dwBytesRead))
      {
        spi.session_id = (DWORD)peb.SessionId;
        spi.debug_flag = (BYTE)peb.BeingDebugged;

        dwBytesRead = 0;
        if (ReadProcessMemory(hProcess, peb.ProcessParameters, &peb_upp, sizeof(UserProcessPar_t), &dwBytesRead))
        {
          if (peb_upp.CommandLine.Length > 0) 
          {
            pwszBuffer = static_cast<WCHAR *>(HeapAlloc(hHeap, HEAP_ZERO_MEMORY, peb_upp.CommandLine.Length));

            if (pwszBuffer)
            {
              if (ReadProcessMemory(hProcess, peb_upp.CommandLine.Buffer, pwszBuffer, peb_upp.CommandLine.Length, &dwBytesRead))
              {
                if (peb_upp.CommandLine.Length >= sizeof(spi.cmd_line))
                {
                  dwBufferSize = sizeof(spi.cmd_line) - sizeof(TCHAR);
                }
                else
                {
                  dwBufferSize = peb_upp.CommandLine.Length;
                }

#if defined(UNICODE) || (_UNICODE)
                StringCbCopyN(spi.cmd_line, sizeof(spi.cmd_line), pwszBuffer, dwBufferSize);
#else
                WideCharToMultiByte(CP_ACP, 0, pwszBuffer, static_cast<int>((dwBufferSize / sizeof(WCHAR))), spi.cmd_line, sizeof(spi.cmd_line), nullptr, nullptr);
#endif
              }
              if (!HeapFree(hHeap, 0, pwszBuffer)) 
              {
                bReturnStatus = false;
                failed_to_free_memory = true;
              }
            }
          }

          if ((peb_upp.ImagePathName.Length > 0) && (failed_to_free_memory == false))
          {
            dwBytesRead = 0;
            pwszBuffer = static_cast<WCHAR *>(HeapAlloc(hHeap, HEAP_ZERO_MEMORY, peb_upp.ImagePathName.Length));
            if (pwszBuffer)
            {
              if (ReadProcessMemory(hProcess, peb_upp.ImagePathName.Buffer, pwszBuffer, peb_upp.ImagePathName.Length, &dwBytesRead))
              {
               if (peb_upp.ImagePathName.Length >= sizeof(spi.image_path))
               {
                  dwBufferSize = sizeof(spi.image_path) - sizeof(TCHAR);
               }
               else
               {
                 dwBufferSize = peb_upp.ImagePathName.Length;
               }
#if defined(UNICODE) || (_UNICODE)
                StringCbCopyN(spi.image_path, sizeof(spi.image_path), pwszBuffer, dwBufferSize);
#else
                WideCharToMultiByte(CP_ACP, 0, pwszBuffer,static_cast<int>((dwBufferSize / sizeof(WCHAR))), spi.image_path, sizeof(spi.image_path), nullptr, nullptr);
#endif
              }
              if (!HeapFree(hHeap, 0, pwszBuffer)) 
              {
                bReturnStatus = false;
                failed_to_free_memory = true;
              }
            }
          }
        }
      }
    }
    if ((spi.process_id == 4) && (failed_to_free_memory == false))
    {
      ExpandEnvironmentStrings(_T("%SystemRoot%\\System32\\ntoskrnl.exe"), spi.image_path, sizeof(spi.image_path));
    }
  }

  if (!HeapFree(hHeap, 0, pbi))
  {
    // failed to free memory
  }
  CloseHandle(hProcess);
  *extended_process_info_ptr_ = spi;

  return bReturnStatus;
}
