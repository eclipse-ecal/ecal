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

#include <windows.h>
#include <iostream>

class Memory
{
 public:
  Memory();
  ~Memory();

  uint64_t GetTotalMemory() const { return total_memory_; }
  uint64_t GetAvailableMemory() const { return available_memory_; }

  void RefreshData();

 private:
  MEMORYSTATUSEX   memory_info_;
  uint64_t         total_memory_;
  uint64_t         available_memory_;
};
