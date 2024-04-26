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

/**
 * @brief  UDP receiver attributes
**/

#pragma once

#include <functional>
#include <string>

namespace eCAL
{
  namespace UDP
  {
    struct SReceiverAttr
    {
      std::string address;
      int         port      = 0;
      bool        broadcast = false;
      bool        loopback  = true;
      int         rcvbuf    = 1024 * 1024;
    };

    using HasSampleCallbackT   = std::function<bool(const std::string& sample_name_)>;
    using ApplySampleCallbackT = std::function<void(const char* serialized_sample_data_, size_t serialized_sample_size_)>;
  }
}
