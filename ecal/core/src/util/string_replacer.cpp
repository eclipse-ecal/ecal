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

#include "string_replacer.h"

#include <iostream>

namespace eCAL
{
  namespace Util
  {
    StringReplacer::StringReplacer() = default;

    std::string StringReplacer::Replace(const std::string& str_)
    {
      const auto& string_it = m_replacer_map.find(str_);
      if (string_it == m_replacer_map.end())
      {
        return str_;
      }

      return string_it->second;
    };

    void StringReplacer::SetMap(const std::unordered_map<std::string, std::string>& map_)
    {
      m_replacer_map = map_;
    };
  }
}