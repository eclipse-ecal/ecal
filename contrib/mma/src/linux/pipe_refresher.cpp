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

#ifdef __unix__

#include <string.h>
#include <iostream>

#include "../include/linux/pipe_refresher.h"

PipeRefresher::PipeRefresher(const std::string& command, uint32_t frequency /*= 1*/)
  : command_(command)
  , frequency_(frequency)
{
  can_send_ = true;
  thread_ = std::thread(&PipeRefresher::FunctionCallback, this);
}

PipeRefresher::~PipeRefresher()
{
  can_send_ = false;
  if (thread_.joinable())
  {
    thread_.join();
  }
    
    
}

void PipeRefresher::AddCallback(MethodCallback callback)
{
  callback_ = callback;
}

void PipeRefresher::RemoveCallback()
{
  callback_ = nullptr;
}

void PipeRefresher::FunctionCallback()
{
  while (can_send_)
  {
    if (callback_ != nullptr)
    {
      auto pipe_result = GetPipeResult();
      callback_(pipe_result, command_);
    }
    for (auto i=0;((i<10)&&(can_send_));++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(frequency_/10));
    }
  }
}

std::string PipeRefresher::GetPipeResult()
{
  std::string result;

  FILE* file = popen(command_.data(), "r");
  char buffer[4096];

  if (file != nullptr)
  {
    while (fgets(buffer, sizeof(buffer), file) != nullptr)
      result += buffer;

    pclose(file);
  }

  strcpy(buffer, "");

  return result;
}
#endif
