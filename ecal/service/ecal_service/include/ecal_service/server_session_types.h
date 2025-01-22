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

#include <functional>
#include <memory>
#include <string>

namespace eCAL
{
  namespace service
  {
    enum class ServerEventType: int
    {
      Connected,          //!< A client has connected successfully.
      Disconnected,       //!< The connection to a client has been closed for any reason.
    };

    using ServerServiceCallbackT = std::function<void(const std::shared_ptr<const std::string>& request, const std::shared_ptr<std::string>& response)>;
    using ServerEventCallbackT   = std::function<void(ServerEventType, const std::string&)>;
  } // namespace service
} // namespace eCAL
