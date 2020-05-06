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

/*
  Please note, the API of eCAL::measurement is not yet stable. It might be subject to API changes in future eCAL versions.
*/

namespace eCAL
{
  namespace measurement
  {
    using ChannelSet = std::set<std::string>;
  
    struct SenderID
    {
      long long ID;
    };


    template<class T>
    struct Frame
    {
      T& message;
      long long send_timestamp;
      long long receive_timestamp;
    };

    template<typename T>
    Frame<T> make_frame(T& message, long long send_timestamp, long long receive_timestamp)
    {
      return { message, send_timestamp, receive_timestamp };
    }

    template<typename T>
    Frame<T> make_frame(T& message, long long timestamp)
    {
      return { message, timestamp, timestamp };
    }

    using BinaryFrame = Frame<std::string>;
 
  }
}