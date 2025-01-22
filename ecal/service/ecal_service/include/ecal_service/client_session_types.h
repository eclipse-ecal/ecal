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

#pragma once

#include <functional>
#include <memory>
#include <string>

#include <ecal_service/error.h>

namespace ecal_service
{
  enum class ClientEventType: int
  {
    Connected,
    Disconnected,
  };
 
  using ClientEventCallbackT    = std::function<void (ClientEventType, const std::string &)>;
  using ClientResponseCallbackT = std::function<void (const ecal_service::Error&, const std::shared_ptr<std::string>&)>;
} // namespace eCAL
