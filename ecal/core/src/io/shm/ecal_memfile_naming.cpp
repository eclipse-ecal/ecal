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

#include "io/shm/ecal_memfile_naming.h"

#include <cstdint>
#include <limits>
#include <random>
#include <sstream>

namespace eCAL
{
  namespace memfile
  {

    std::string BuildRandomMemFileName(const std::string& base_name)
    {
      static std::random_device rd;
      static std::uniform_int_distribution<uint32_t> dist(0, std::numeric_limits<uint32_t>::max());

      std::stringstream out;
      out << base_name << std::hex << dist(rd);

      return out.str();
    }
  }
}