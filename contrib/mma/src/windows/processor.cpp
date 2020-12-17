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

#include <windows.h>
#include <malloc.h>    
#include <stdio.h>
#include <tchar.h>

#include "../include/windows/processor.h"

Processor::Processor() : 
  processor_(NULL), 
  nr_cores(0)
{
}

Processor::~Processor()
{
}

void Processor::RefreshData(const HQUERY& h_query, PDH_STATUS& status)
{
  // adding the processor counter to the HQuery
  status = PdhAddEnglishCounter(h_query, "\\Processor(_Total)\\% Processor Time", 0, &processor_);

  if (status != ERROR_SUCCESS)
  {
    Logger::getLogger()->Log("PdhAddCounter [CPU] failed with code " + std::to_string(status));
  }
}

DWORD Processor::GetNumbersOfCpuCores( )
{
  LPFN_GLPI glpi;
  BOOL done = FALSE;
  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
  unsigned long  returnLength = 0;
  DWORD processor_core_count = 0;
  DWORD byteOffset = 0;

  glpi = (LPFN_GLPI)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetLogicalProcessorInformation");
  if (NULL == glpi)
  {
    Logger::getLogger()->Log("GetLogicalProcessorInformation is not supported");
    return (1);
  }

  while (!done)
  {
    DWORD rc = glpi(buffer, &returnLength);

    if (FALSE == rc)
    {
      if (GetLastError( ) == ERROR_INSUFFICIENT_BUFFER)
      {
        (buffer) ? (delete buffer) : false;
        buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(returnLength);

        if (NULL == buffer)
        {
          Logger::getLogger()->Log("Error: Allocation failure");
          return (2);
        }
      }
      else
      {
        Logger::getLogger()->Log("Error: " + GetLastError());
        return (3);
      }
    }
    else
    {
      done = TRUE;
    }
  }

  ptr = buffer;

  while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength)
  {
    switch (ptr->Relationship)
    {
    case RelationProcessorCore:
      processor_core_count++;
      break;
    default:
      break;
    }
    byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
    ptr++;
  }
  delete buffer;
  return processor_core_count;
}
