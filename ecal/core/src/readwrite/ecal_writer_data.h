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
 * @brief  data writer struct
**/

#pragma once

#include <cstddef>

namespace eCAL
{
  struct SWriterAttr
  {
    size_t       len                    = 0;
    long long    clock                  = 0;
    size_t       hash                   = 0;
    long long    time                   = 0;
    bool         loopback               = false;
    bool         zero_copy              = false;
    long long    acknowledge_timeout_ms = 0;
  };
}
