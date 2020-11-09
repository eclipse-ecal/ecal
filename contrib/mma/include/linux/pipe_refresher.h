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

#ifdef __unix__

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

class PipeRefresher
{
 public:
  explicit PipeRefresher(const std::string& command, uint32_t frequency = 1);
  ~PipeRefresher();

  typedef std::function<void(const std::string&, const std::string&)> MethodCallback;
  void AddCallback(MethodCallback callback);
  void RemoveCallback();

 private:
  MethodCallback callback_;
  std::string command_;

  std::atomic<bool> can_send_;
  uint32_t frequency_;

  std::thread thread_;

  void FunctionCallback();
  std::string GetPipeResult();
};
#endif
