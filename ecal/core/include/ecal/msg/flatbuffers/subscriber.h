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
 * @brief  eCAL subscriber interface for google::flatbuffers message definitions
**/

#pragma once

#include <ecal/msg/subscriber.h>

namespace eCAL
{
  namespace internal
  {
    template <typename T>
    class FlatbuffersDeserializer
    {
    public:
      static SDataTypeInformation GetDataTypeInformation()
      {
        SDataTypeInformation topic_info;
        topic_info.encoding = "flatb";
        // empty type, empty descriptor
        return topic_info;
      }

      static bool Deserialize(T& msg_, const void* buffer_, size_t size_)
      {
        msg_.PushFlatBuffer(static_cast<const uint8_t*>(buffer_), static_cast<int>(size_));
        return(true);
      }
    };
  }

  namespace flatbuffers
  {

    /**
     * @brief  eCAL google::flatbuffers subscriber class.
     *
     * Subscriber template  class for google::flatbuffers messages. For details see documentation of CSubscriber class.
     *
    **/
    template <typename T>
    using CSubscriber = CMessageSubscriber<T, internal::FlatbuffersDeserializer<T>>;

    /** @example monster_rec.cpp
    * This is an example how to use eCAL::CSubscriber to receive goggle::flatbuffers data with eCAL. To send the data, see @ref monster_snd.cpp .
    */
  }
}