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
 * @brief  System usage monitoring
**/

#include <ecal/ecal_os.h>

#ifdef ECAL_OS_WINDOWS

#include "ecal_win_main.h"

class CpuUsage
{
public:
  CpuUsage(void);

  short  GetUsage();
private:
  ULONGLONG SubtractTimes(const FILETIME& ftA, const FILETIME& ftB);
  bool EnoughTimePassed();
  inline bool IsFirstRun() const { return (m_dwLastRun == 0); }

  short         m_nCpuUsage;
  volatile LONG m_lRunCount;
  ULONGLONG     m_dwLastRun;

  // system total times
  FILETIME m_ftPrevSysKernel;
  FILETIME m_ftPrevSysUser;

  // process times
  FILETIME m_ftPrevProcKernel;
  FILETIME m_ftPrevProcUser;
};


CpuUsage::CpuUsage(void)
  :m_nCpuUsage(-1)
  ,m_dwLastRun(0)
  ,m_lRunCount(0)
{
  ZeroMemory(&m_ftPrevSysKernel, sizeof(FILETIME));
  ZeroMemory(&m_ftPrevSysUser, sizeof(FILETIME));

  ZeroMemory(&m_ftPrevProcKernel, sizeof(FILETIME));
  ZeroMemory(&m_ftPrevProcUser, sizeof(FILETIME));

}

/**********************************************
* CpuUsage::GetUsage
* returns the percent of the CPU that this process
* has used since the last time the method was called.
* If there is not enough information, -1 is returned.
* If the method is recalled to quickly, the previous value
* is returned.
***********************************************/
short CpuUsage::GetUsage()
{
  //create a local copy to protect against race conditions in setting the 
  //member variable
  short nCpuCopy = m_nCpuUsage;
  if (::InterlockedIncrement(&m_lRunCount) == 1)
  {
    /*
    If this is called too often, the measurement itself will greatly affect the
    results.
    */

    if (!EnoughTimePassed())
    {
      ::InterlockedDecrement(&m_lRunCount);
      return nCpuCopy;
    }

    FILETIME ftSysIdle, ftSysKernel, ftSysUser;
    FILETIME ftProcCreation, ftProcExit, ftProcKernel, ftProcUser;

    if (!GetSystemTimes(&ftSysIdle, &ftSysKernel, &ftSysUser) ||
      !GetProcessTimes(GetCurrentProcess(), &ftProcCreation, &ftProcExit, &ftProcKernel, &ftProcUser))
    {
      ::InterlockedDecrement(&m_lRunCount);
      return nCpuCopy;
    }

    if (!IsFirstRun())
    {
      /*
      CPU usage is calculated by getting the total amount of time the system has operated
      since the last measurement (made up of kernel + user) and the total
      amount of time the process has run (kernel + user).
      */
      ULONGLONG ftSysKernelDiff = SubtractTimes(ftSysKernel, m_ftPrevSysKernel);
      ULONGLONG ftSysUserDiff = SubtractTimes(ftSysUser, m_ftPrevSysUser);

      ULONGLONG ftProcKernelDiff = SubtractTimes(ftProcKernel, m_ftPrevProcKernel);
      ULONGLONG ftProcUserDiff = SubtractTimes(ftProcUser, m_ftPrevProcUser);

      ULONGLONG nTotalSys =  ftSysKernelDiff + ftSysUserDiff;
      ULONGLONG nTotalProc = ftProcKernelDiff + ftProcUserDiff;

      if (nTotalSys > 0)
      {
        m_nCpuUsage = (short)((100.0 * nTotalProc) / nTotalSys);
      }
    }

    m_ftPrevSysKernel = ftSysKernel;
    m_ftPrevSysUser = ftSysUser;
    m_ftPrevProcKernel = ftProcKernel;
    m_ftPrevProcUser = ftProcUser;

#if defined(__MINGW32__)
    m_dwLastRun = GetTickCount();
#else
  #if defined(_WIN64)
    m_dwLastRun = GetTickCount64();
  #else
    m_dwLastRun = GetTickCount();
  #endif
#endif

    nCpuCopy = m_nCpuUsage;
  }

  ::InterlockedDecrement(&m_lRunCount);

  return nCpuCopy;
}

ULONGLONG CpuUsage::SubtractTimes(const FILETIME& ftA, const FILETIME& ftB)
{
  LARGE_INTEGER a, b;
  a.LowPart = ftA.dwLowDateTime;
  a.HighPart = ftA.dwHighDateTime;

  b.LowPart = ftB.dwLowDateTime;
  b.HighPart = ftB.dwHighDateTime;

  return a.QuadPart - b.QuadPart;
}

bool CpuUsage::EnoughTimePassed()
{
  const int minElapsedMS = 250; //milliseconds

#if defined(__MINGW32__)
  ULONGLONG dwCurrentTickCount = GetTickCount();
#else
  #if defined(_WIN64)
  ULONGLONG dwCurrentTickCount = GetTickCount64();
  #else
  ULONGLONG dwCurrentTickCount = GetTickCount();
  #endif
#endif

  return (dwCurrentTickCount - m_dwLastRun) > minElapsedMS; 
}

static CpuUsage g_cpu_usage;

float GetCPULoad()
{
  static float usage = 0.0f;
  short cpu_usage = g_cpu_usage.GetUsage();
  // -1 means could not be evaluated yet, do not use that value ..
  if (cpu_usage != -1) usage = 0.98f * usage + 0.02f * static_cast<float>(cpu_usage);
  return(usage * 0.01f);
}

#endif /* ECAL_OS_WINDOWS */

#ifdef ECAL_OS_LINUX

float GetCPULoad()
{
  return(0.0f);
}

#endif /* ECAL_OS_LINUX */
