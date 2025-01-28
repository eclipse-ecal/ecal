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
 * @brief  eCAL publisher interface for std::string message definitions
**/

#pragma once

#include <cstddef>
#include <ecal/deprecate.h>
#include <ecal/msg/publisher.h>

#include <string>
#include <memory.h>

namespace eCAL
{
  namespace string
  {
    /**
     * @brief  eCAL publisher class for standard strings.
     *
     * Publisher class for STL standard strings. For details see documentation of CPublisher class.
     *
    **/
    template <typename T>
    class CPublisher : public CMsgPublisher<T>
    {
    public:
      /**
       * @brief  Constructor.
      **/
      CPublisher() : CMsgPublisher<T>()
      {
      }

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
        data_type_info.encoding = "base";
        data_type_info.name     = "std::string";
        // empty descriptor
        return data_type_info;
      }
      
      /**
       * @brief  Get size of the string object.
       *
       * @param msg_  The message object.
       *
       * @return  String size.
      **/
      size_t GetSize(const T& msg_) const override
      {
        return(msg_.size());
      }

      /**
       * @brief  Copy the string object into a pre-allocated char buffer.
       *
       * @param       msg_     The message object.
       * @param [out] buffer_  Target buffer.
       * @param       size_    Target buffer size.
       *
       * @return  True if it succeeds, false if it fails.
      **/
      bool Serialize(const T& msg_, char* buffer_, size_t size_) const override
      {
        if (msg_.size() <= size_)
        {
          memcpy(buffer_, msg_.c_str(), msg_.size());
          return(true);
        }
        return(false);
      }
    };
    /** @example minimal_snd.cpp
     * This is an example how to use eCAL::string::CPublisher to send a std::string with eCAL. To receive the strings, see @ref minimal_rec.cpp .
    **/
  }
}
