/* ========================= eCAL LICENSE =================================
 *
 * Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
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
#include <unordered_map>

namespace eCAL
{
  namespace Config
  {
    using ReMap = std::unordered_map<std::string, std::string>;
    struct LaunchConfiguration
    {
      ReMap publisher_topics;
      ReMap subscriber_topics;
      ReMap client_calls;
      ReMap server_methods;
    };
  } // Config
} // eCAL