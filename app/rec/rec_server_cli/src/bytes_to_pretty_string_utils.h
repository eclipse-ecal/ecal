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

#include <vector>
#include <string>

namespace eCAL
{
  namespace rec_cli
  {
    static std::string bytesToPrettyString(std::uint64_t bytes)
    {
      static const std::vector<std::pair<std::uint64_t, std::string>> units = 
      {
        { std::uint64_t(1) <<  0, "Bytes" },
        { std::uint64_t(1) << 10, "KiB" },
        { std::uint64_t(1) << 20, "MiB" },
        { std::uint64_t(1) << 30, "GiB" },
        { std::uint64_t(1) << 40, "TiB" },
        { std::uint64_t(1) << 50, "PiB" },
        { std::uint64_t(1) << 60, "EiB" },
      };

      size_t proper_unit_index = 0;

      for (size_t i = units.size(); i > 0; i--)
      {
        if (units[i - 1].first < bytes)
        {
          proper_unit_index = i - 1;
          break;
        }
      }

      return std::to_string((double)bytes / double(units[proper_unit_index].first)) + " " + units[proper_unit_index].second;
    }
  }
}
