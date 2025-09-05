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

/**
 * @brief  Definition of types and functions to be used when working with atomic variables.
**/

#pragma once

#include <memory>
#include <string>

namespace eCAL
{
  namespace Types
  {
    namespace Atomic
    {
      using StringSPtr = std::shared_ptr<std::string>;

      /**
       * @brief This functions stores atomically a new value
       *        by creating a new shared pointer for the variable.
       * 
       * @param string_sptr_ String shared pointer
       * @param new_value_   Value to be set into the shared pointer
       */
      void Store(StringSPtr& string_sptr_, const std::string& new_value_);

      /**
       * @brief This functions loads atomically values out of a shared pointer.
       * 
       * @param string_sptr_ String shared pointer
       * 
       * @returns Copy of the value of the shared pointer as std::string.
       */
      std::string Load(const StringSPtr& string_sptr_);
    } // Atomic
  } // Types
} // eCAL