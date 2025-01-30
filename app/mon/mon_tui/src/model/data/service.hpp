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
#include <vector>
#include <cstdint>

struct Service
{
  struct Method
  {
    std::string name;
    std::string request_type;
    std::string response_type;
    int64_t call_count;
  };

  std::string name;
  std::string id;
  std::string host_name;
  std::string process_name;
  int32_t process_id;
  std::string unit_name;
  int32_t registration_clock;
  uint32_t tcp_port;
  std::vector<Method> methods;
};
