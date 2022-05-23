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

#include <ecal/msg/proto/message.h>
#include <ecal/measurement/omeasurement.h>

namespace eCAL
{
  namespace measurement
  {
    template<typename T>
    struct OChannelType<T, typename std::enable_if_t<std::is_base_of_v<google::protobuf::Message, T>>> { using type = OBaseChannel<T, eCAL::message::protobuf::MessageProvider>; };
  }
}

