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

#include "rec_server_impl.h"

namespace eCAL
{
  namespace rec_server
  {
    namespace config
    {
      static constexpr int NATIVE_CONFIG_VERSION = 3;

      bool writeConfigToFile(const eCAL::rec_server::RecServerImpl& rec_server, const std::string& path);

      bool readConfigFromFile(eCAL::rec_server::RecServerImpl& rec_server, const std::string& path, int* version = nullptr);
    }
  }
}

