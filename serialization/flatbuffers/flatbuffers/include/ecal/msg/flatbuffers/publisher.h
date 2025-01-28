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
 * @brief  eCAL publisher interface for google::flatbuffers message definitions
**/

#pragma once

#include <ecal/msg/publisher.h>

namespace eCAL
{
  namespace flatbuffers
  {
    /**
     * @brief eCAL google::flatbuffers publisher class.
     *
     * Publisher template  class for goggle::flatbuffers messages. For details see documentation of CPublisher class.
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
        data_type_info.encoding = "flatb";
        // empty type, empty descriptor
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
        return((size_t)msg_.GetSize());
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
        if (size_ < msg_.GetSize()) return(false);
        memcpy(buffer_, msg_.GetBufferPointer(), msg_.GetSize());
        return(true);
      }
    };
    /** @example monster_snd.cpp
    * This is an example how to use eCAL::CPublisher to send goggle::flatbuffers data with eCAL. To receive the data, see @ref monster_rec.cpp .
    */
  }
}
