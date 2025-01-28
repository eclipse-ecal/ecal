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
 * @file   publisher.h
 * @brief  eCAL publisher interface for messagepack message definitions
**/

#pragma once

#include <ecal/msg/publisher.h>

#include <msgpack.hpp>
#include <sstream>

namespace eCAL
{
  namespace messagepack
  {
    /**
     * @brief eCAL msgpack publisher class.
     *
     * Publisher template  class for msgpack messages. For details see documentation of CPublisher class.
     *
    **/
    template <typename T>
    class CPublisher : public CMsgPublisher<T>
    {
    public:
      /**
       * @brief  Constructor.
       *
       * @param topic_name_  Unique topic name.
       * @param config_      Optional configuration parameters.
      **/
      explicit CPublisher(const std::string& topic_name_, const eCAL::Publisher::Configuration& config_ = GetPublisherConfiguration()) : CMsgPublisher<T>(topic_name_, GetDataTypeInformation(), config_)
      {
      }

      /**
       * @brief  Constructor.
       *
       * @param topic_name_      Unique topic name.
       * @param event_callback_  The publisher event callback funtion.
       * @param config_          Optional configuration parameters.
      **/
      explicit CPublisher(const std::string& topic_name_, const eCAL::PubEventCallbackT& event_callback_, const eCAL::Publisher::Configuration& config_ = GetPublisherConfiguration()) : CMsgPublisher<T>(topic_name_, GetDataTypeInformation(), event_callback_, config_)
      {
      }

      /**
      * @brief  Copy Constructor is not available.
      **/
      CPublisher(const CPublisher&) = delete;

      /**
      * @brief  Copy Constructor is not available.
      **/
      CPublisher& operator=(const CPublisher&) = delete;

      /**
      * @brief  Move Constructor
      **/
      CPublisher(CPublisher&&) = default;

      /**
      * @brief  Move assignment
      **/
      CPublisher& operator=(CPublisher&&) = default;

    private:
      /**
      * @brief   Get topic information of the message.
      *
      * @return  Topic information.
      **/
      SDataTypeInformation GetDataTypeInformation() const override
      {
        SDataTypeInformation data_type_info;
        data_type_info.encoding = "mpack";
        // empty descriptor, empty descriptor
        return data_type_info;
      }

      /**
       * @brief  Get size for serialized message object.
       *
       * @param msg_  The message object.
       *
       * @return  Message size.
      **/
      size_t GetSize(const T& msg_) const
      {
        msgpack::sbuffer buf;
        msgpack::pack(&buf, msg_);
        return(buf.size());
      }

      /**
       * @brief  Serialize the message object into a preallocated char buffer.
       *
       * @param       msg_     The message object.
       * @param [out] buffer_  Target buffer.
       * @param       size_    Target buffer size.
       *
       * @return  True if it succeeds, false if it fails.
      **/
      bool Serialize(const T& msg_, char* buffer_, size_t size_) const
      {
        std::stringstream buffer;
        msgpack::pack(buffer, msg_);
        buffer.seekg(0, std::ios::end);
        size_t size = static_cast<size_t>(buffer.tellg());
        if (size > size_) return(false);
        buffer.seekg(0, std::ios::beg);
        buffer.read(buffer_, size_);
        return(true);
      }
    };
    /** @example address_snd.cpp
    * This is an example how to use eCAL::CPublisher to send msgpack data with eCAL. To receive the data, see @ref address_rec.cpp .
    */
  }
}
