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

#include "atomics.h"

namespace eCAL
{
  namespace Types
  {
    namespace Atomic
    {    
      void Update(StringSPtr& atomic_string_, const std::string& new_value_)
      {
        std::atomic_store(&atomic_string_, std::make_shared<std::string>(new_value_));
      }

      std::string Read(const StringSPtr& atomic_string_)
      {
        auto ptr = std::atomic_load(&atomic_string_);
        return ptr ? *ptr : std::string{};
      }
    } // Atomic
  } // Types
} // eCAL