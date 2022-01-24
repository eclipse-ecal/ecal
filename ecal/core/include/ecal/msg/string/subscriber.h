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
 * @brief  eCAL subscriber interface for std::string message definitions
**/

#pragma once

#include <ecal/msg/subscriber.h>

#include <string>
#include <memory.h>

namespace eCAL
{
  namespace string
  {
    /**
     * @brief  eCAL subscriber class for standard strings.
     *
     * Subscriber class for STL standard strings. For details see documentation of CSubscriber class.
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
       * @brief  Get type name.
       *
       * @return  Always returns "base:std::string".
      **/
      std::string GetTypeName() const
      {
        return("base:std::string");
      }

    private:
      /**
       * @brief  Get description.
       *
       * @return  Always returns empty string.
      **/
      std::string GetDescription() const
      {
        return("");
      }

      /**
       * @brief  Copy the string object from char buffer.
       *
       * @param [out] msg_     The message object.
       * @param       buffer_  Target buffer.
       * @param       size_    Target buffer size.
       *
       * @return  True if it succeeds, false if it fails.
      **/
      bool Deserialize(T& msg_, const void* buffer_, size_t size_) const
      {
        msg_ = std::string(static_cast<const char*>(buffer_), size_);
        return true;
      }

    };
    /** @example minimal_rec.cpp
    * This is an example how to use eCAL::CSubscriber to receive a std::string with eCAL. To send the strings, see @ref minimal_snd.cpp .
    */
  }
}
