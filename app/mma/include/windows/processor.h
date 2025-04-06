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

#pragma once

#include <Pdh.h>
#include <string>

#include "../logger.h"

typedef BOOL(WINAPI *LPFN_GLPI)(
  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION,
  PDWORD);


class Processor
{
 public:
  Processor();
  ~Processor();

  void RefreshData(const HQUERY& h_query, PDH_STATUS& status);
  PDH_HCOUNTER GetProcessor() const { return processor_; }
  DWORD GetNumbersOfCpuCores( );

 private:
  PDH_HCOUNTER processor_;
  DWORD        nr_cores;
};
