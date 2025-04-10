/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
#include <ecalhdf5/eh5_types.h>

namespace eCAL
{
  namespace eh5
  {
    std::string GetEscapedTopicname(const std::string& input);
    SChannel    GetEscapedTopicname(const SChannel& input);
    SWriteEntry GetEscapedEntry(const SWriteEntry& input);
    std::string GetEscapedFilename(const std::string& input);
    std::string GetUnescapedString(const std::string& input);
  }
}
