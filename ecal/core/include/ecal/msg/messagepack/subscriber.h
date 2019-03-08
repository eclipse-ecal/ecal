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
 * @file   subscriber.h
 * @brief  eCAL subscriber interface for messagepack message definitions
**/

#pragma once

#include <ecal/msg/subscriber.h>

#include <msgpack.hpp>
#include <sstream>

namespace eCAL
{
  namespace messagepack
  {
    /**
     * @brief  eCAL msgpack subscriber class.
     *
     * Subscriber template  class for msgpack messages. For details see documentation of CSubscriber class.
     *
    **/
    template <typename T>
    class CSubscriber : public CMsgSubscriber<T>
    {
    public:
      /**
       * @brief  Constructor.
      **/
      CSubscriber() : CMsgSubscriber<T>()
      {
      }

      /**
       * @brief  Constructor.
       *
       * @param topic_name_  Unique topic name.
      **/
      CSubscriber(const std::string& topic_name_) : CMsgSubscriber<T>(topic_name_, GetTypeName(), GetDescription())
      {
      }

      /**
      * @brief  Copy Constructor is not available.
      **/
      CSubscriber(const CSubscriber&) = delete;

      /**
      * @brief  Copy Constructor is not available.
      **/
      CSubscriber& operator=(const CSubscriber&) = delete;

      /**
      * @brief  Move Constructor
      **/
      CSubscriber(CSubscriber&&) = default;

      /**
      * @brief  Move assignment
      **/
      CSubscriber& operator=(CSubscriber&&) = default;

      /**
       * @brief  Creates this object.
       *
       * @param topic_name_  Unique topic name.
       *
       * @return  True if it succeeds, false if it fails.
      **/
      bool Create(const std::string& topic_name_)
      {
        return(CMsgSubscriber<T>::Create(topic_name_, GetTypeName(), GetDescription()));
      }

      /**
       * @brief  Get type name of the msgpack message.
       *
       * @return  Type name.
      **/
      std::string GetTypeName() const
      {
        return("mpack:");
      }

    private:
      /**
       * @brief  Get file descriptor string of the msgpack message.
       *
       * @return  Description string.
      **/
      std::string GetDescription() const
      {
        return("");
      }

      /**
       * @brief  Deserialize the message object from a message buffer.
       *
       * @param [out] msg_     The message object.
       * @param       buffer_  Source buffer.
       * @param       size_    Source buffer size.
       *
       * @return  True if it succeeds, false if it fails.
      **/
      bool Deserialize(T& msg_, const void* buffer_, size_t size_) const
      {
        msgpack::unpacked ubuffer;
        msgpack::unpack(ubuffer, static_cast<const char*>(buffer_), size_);
        msgpack::object deserialized = ubuffer.get();
        deserialized.convert(msg_);
        return(true);
      }

    };
    /** @example address_rec.cpp
    * This is an example how to use eCAL::CSubscriber to receive msgpack data with eCAL. To send the data, see @ref address_snd.cpp .
    */
  }
}
