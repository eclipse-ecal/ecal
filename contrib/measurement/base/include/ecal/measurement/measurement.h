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

#include <set>
#include <string>
#include <ecal/measurement/base/types.h>

/*
  Please note, the API of eCAL::measurement is not yet stable. It might be subject to API changes in future eCAL versions.
*/

namespace eCAL
{
  namespace measurement
  {
    using ChannelSet = std::set<eCAL::experimental::measurement::base::Channel>;
  
    // A frame is only a leightweight wrapper around a datatype, which also contains send / receive timestamp
    // It is non-owning and should be treated as a view.
    template<class T>
    struct Frame
    {
      const T& message;
      long long send_timestamp;
      long long receive_timestamp;
    };

    template<typename T>
    Frame<T> make_frame(const T& message, long long send_timestamp, long long receive_timestamp)
    {
      return { message, send_timestamp, receive_timestamp };
    }

    template<typename T>
    Frame<T> make_frame(const T& message, long long timestamp)
    {
      return { message, timestamp, timestamp };
    }

    // This is a frame that holds binary data.
    using BinaryFrame = Frame<std::string>;
  }
}
