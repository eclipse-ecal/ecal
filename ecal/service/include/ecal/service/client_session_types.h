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

#include <ecal/service/error.h>

#include "ecal/cimpl/ecal_callback_cimpl.h"

namespace eCAL
{
  namespace service
  {
    using ClientEventCallbackT    = std::function<void (eCAL_Client_Event, const std::string &)>;  // TODO: Ask Rex what the "eCAL Client Event timeout" is. Should it maybe be fired, when the connection establishement failed? Currently, in that case no event whatsoever is fired, as there is no "Connection failed" event.
    using ClientResponseCallbackT = std::function<void (const eCAL::service::Error&, const std::shared_ptr<std::string>&)>;
  } // namespace service
} // namespace eCAL
