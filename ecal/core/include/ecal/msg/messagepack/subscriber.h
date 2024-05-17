/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
 * @file   subscriber.h
 * @brief  eCAL subscriber interface for messagepack message definitions
**/

#pragma once

#include <ecal/msg/subscriber.h>

#include <msgpack.hpp>
#include <sstream>

namespace eCAL
{
  namespace internal
  {
    template <typename T>
    class MessagePackDeserializer
    {
    public:
      static SDataTypeInformation GetDataTypeInformation()
      {
        SDataTypeInformation topic_info;
        topic_info.encoding = "mpack";
        // empty descriptor, empty descriptor
        return topic_info;
      }

      static bool Deserialize(T& msg_, const void* buffer_, size_t size_, const SDataTypeInformation&  /*datatype_info_*/)
      {
        msgpack::unpacked ubuffer;
        msgpack::unpack(ubuffer, static_cast<const char*>(buffer_), size_);
        msgpack::object deserialized = ubuffer.get();
        deserialized.convert(msg_);
        return(true);
      }
    };
  }

  namespace messagepack
  {

    /**
     * @brief  eCAL msgpack subscriber class.
     *
     * Subscriber template  class for msgpack messages. For details see documentation of CSubscriber class.
     *
    **/
    template <typename T>
    using CSubscriber = CMessageSubscriber<T, internal::MessagePackDeserializer<T>>;

    /** @example address_rec.cpp
    * This is an example how to use eCAL::CSubscriber to receive msgpack data with eCAL. To send the data, see @ref address_snd.cpp .
    */
  }
}

