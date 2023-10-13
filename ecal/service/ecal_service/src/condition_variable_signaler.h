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

#include <condition_variable>
#include <mutex>

// Class that signals a condition variable when it is destroyed
class ConditionVariableSignaler
{
public:
  ConditionVariableSignaler(std::condition_variable& condition_variable, std::mutex& mutex, bool& is_signaled)
    : mutex_             (mutex)
    , condition_variable_(condition_variable)
    , is_signaled_       (is_signaled)
  {}

  ~ConditionVariableSignaler()
  {
    const std::lock_guard<std::mutex> lock(mutex_);
    is_signaled_ = true;
    condition_variable_.notify_all();
  }

  // Delete copy constructor and assignment operator
  ConditionVariableSignaler(const ConditionVariableSignaler&)            = delete;
  ConditionVariableSignaler& operator=(const ConditionVariableSignaler&) = delete;

  // Default move constructor and assignment operator
  ConditionVariableSignaler(ConditionVariableSignaler&&)            = default;
  ConditionVariableSignaler& operator=(ConditionVariableSignaler&&) = default;

  private:
    std::mutex&              mutex_;
    std::condition_variable& condition_variable_;
    bool&                    is_signaled_;
};
