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

#pragma once

#include <string>
#include <map>
#include <vector>
#include <windows.h>

#include <tchar.h>
#include <winternl.h>
#include <psapi.h>

#define STRSAFE_LIB
#include <strsafe.h>

#pragma comment(lib, "strsafe.lib")
#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "psapi.lib")

#ifndef NTSTATUS
#define LONG NTSTATUS
#endif

#include "../logger.h"

typedef struct ProcessInfo_s
{
  std::string        name;               // path and name of the process
  std::string        command_line;       // command line of the process
  unsigned long long cpu_time_kernel;    // cpu time of the process in kernel
  unsigned long long cpu_time_user;      // cpu time of the process in user mode
  unsigned long long cpu_creation_time;  // cpu creation time
  float              cpu_consumption;    // cpu consumption in xx.yy%
  unsigned long int  memory_current;     // current memory consumption of the process in bytes
  unsigned long int  memory_peak;        // peak memory consumption of the process
  bool               debug_flag;         // indicating the process is in debug mode
  bool               is_new;             // indicating a new element
  bool               not_accessible;     // indicating that this process is not accessible
  unsigned long long last_time_measured; // last time the cpu times were measured

  ProcessInfo_s() :
    name(""),
    command_line(""),
    cpu_time_kernel(0),
    cpu_time_user(0),
    cpu_creation_time(0),
    cpu_consumption(0),
    memory_current(0),
    memory_peak(0),
    debug_flag(false),
    is_new(true),
    not_accessible(false),
    last_time_measured(0)
  {}
}ProcessInfo_t, *ProcessInfo_ptr_t;

typedef std::map<unsigned int, ProcessInfo_t> ProcessMap_t, *ProcessMap_ptr_t;




class Processes {
public:
  Processes(void);
  ~Processes();
  
  bool RefeshData(void);

  ProcessMap_ptr_t GetProcessesInfo(void);


private:
  typedef std::pair<unsigned int, ProcessInfo_t> ProcessPair_t, *ProcessPair_ptr_t;

  typedef std::vector<DWORD>  ProcessIdList_t, *ProcessIdList_ptr_t;

  typedef ULONG PostProcessInitRoutine_t;

  typedef struct ProcessLdrData_s
  {
    BYTE Reserved1[8];
    PVOID Reserved2[3];
    LIST_ENTRY InMemoryOrderModuleList;
  } ProcessLdrData_t, *ProcessLdrData_ptr_t;

  typedef struct UserProcessPar_s 
  {
    BYTE Reserved1[16];
    PVOID Reserved2[10];
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
  } UserProcessPar_t, *UserProcessPar_ptr_t;

  typedef struct BasicInfoStruct_s 
  {
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[1];
    PVOID Reserved3[2];
    ProcessLdrData_ptr_t Ldr;
    UserProcessPar_ptr_t ProcessParameters;
    BYTE Reserved4[104];
    PVOID Reserved5[52];
    PostProcessInitRoutine_t PostProcessInitRoutine;
    BYTE Reserved6[128];
    PVOID Reserved7[1];
    ULONG SessionId;
  } BasicInfoStruct_t, *BasicInfoStruct_ptr_t;

  typedef struct BasisProcessInfo_s 
  {
    LONG ExitStatus;
    BasicInfoStruct_ptr_t PebBaseAddress;
    ULONG_PTR AffinityMask;
    LONG BasePriority;
    ULONG_PTR UniqueProcessId;
    ULONG_PTR InheritedFromUniqueProcessId;
  } BasisProcessInfo_t, *BasisProcessInfo_ptr_t;

  typedef NTSTATUS(NTAPI *pfnNtQueryInformationProcess)( HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength );

  typedef struct ExtendedProcessInfo_s
  {
    DWORD	process_id;
    DWORD	parent_pid;
    DWORD	session_id;
    DWORD	peb_base_address;
    DWORD	affinity_mask;
    LONG	base_priority;
    LONG	exit_status;
    BYTE	debug_flag;
    TCHAR	image_path[32767L];
    TCHAR	cmd_line[32767L];
  } ExtendedProcessInfo_t, *ExtendedProcessInfo_ptr_t;

  typedef enum WorkMode_e
  {
    DISABLED = 0,
    REDUCED  = 1,
    FULLY    = 2
  } WorkMode_t;


  bool UpdateProcessMap(void);
  void AlignProcessMapWithNewProcessList();
  bool GetListOfProcessIDs(void);
  bool LoadNTdllFunctionality(void);
  void FreeNTdllFunctionality(void);
  bool EnableTokenPrivilege(void);
  bool GetExtendedProcessInfo(const DWORD _pid, ExtendedProcessInfo_ptr_t extended_process_info_ptr_);


  WorkMode_t   work_mode;

  ProcessMap_t process_map;

  ProcessIdList_t process_list;

  HMODULE ntdll_module_hdl;

  pfnNtQueryInformationProcess gNtQueryInformationProcess;

};