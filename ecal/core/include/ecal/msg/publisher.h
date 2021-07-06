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
 * @file   ecal_publisher.h
 * @brief  eCAL message publisher interface
**/

#pragma once

#include <ecal/ecal_publisher.h>

#include <string>
#include <vector>
#include <functional>
#include <assert.h>
#include <string.h>

namespace eCAL
{
  /**
   * @brief eCAL abstract message publisher class.
   *
   * Abstract publisher template class for messages. For details see documentation of CPublisher class.
   * 
  **/
  template <typename T>
  class CMsgPublisher : public CPublisher
  {
  public:
    /**
     * @brief  Constructor. 
    **/
    CMsgPublisher() : CPublisher()
    {
    }

    /**
     * @brief  Constructor. 
     *
     * @param topic_name_  Unique topic name. 
     * @param topic_type_  Type name (optional for type checking). 
     * @param topic_desc_  Type description (optional for description checking). 
    **/
    CMsgPublisher(const std::string& topic_name_, const std::string& topic_type_ = "", const std::string& topic_desc_ = "") : CPublisher(topic_name_, topic_type_, topic_desc_)
    {
    }

    /**
    * @brief  Copy Constructor is not available.
    **/
    CMsgPublisher(const CMsgPublisher&) = delete;

    /**
    * @brief  Copy Constructor is not available.
    **/
    CMsgPublisher& operator=(const CMsgPublisher&) = delete;

    /**
    * @brief  Move Constructor
    **/
    CMsgPublisher(CMsgPublisher&&) = default;

    /**
    * @brief  Move assignment
    **/
    CMsgPublisher& operator=(CMsgPublisher&&) = default;

    virtual ~CMsgPublisher() = default;

    /**
     * @brief  Creates this object. 
     *
     * @param topic_name_   Unique topic name. 
     * @param topic_type_   Type name (optional for type checking). 
     * @param topic_desc_   Type description (optional for description checking). 
     *
     * @return  True if it succeeds, false if it fails. 
    **/
    bool Create(const std::string& topic_name_, const std::string& topic_type_ = "", const std::string& topic_desc_ = "")
    {
      return(CPublisher::Create(topic_name_, topic_type_, topic_desc_));
    }

    /**
     * @brief  Destroys this object. 
     *
     * @return  True if it succeeds, false if it fails. 
    **/
    bool Destroy()
    {
      return(CPublisher::Destroy());
    }

    /**
     * @brief  Send serialized message. 
     *
     * @param msg_   The message object. 
     * @param time_  Optional time stamp. 
     *
     * @return  Number of bytes sent. 
    **/
    size_t Send(const T& msg_, long long time_ = -1)
    {
      // this is an optimization ...
      // if there is no subscription we do not waste time for
      // serialization but we send an empty payload
      // to still do some statistics like message clock
      // counting and frequency calculation for the monitoring layer
      if (!IsSubscribed())
      {
        return(CPublisher::Send(nullptr, 0));
      }

      // if we have a subscription allocate memory for the
      // binary stream, serialize the message into the
      // buffer and finally send it with a binary publisher
      size_t size = GetSize(msg_);
      if(size > 0)
      {
        m_buffer.resize(size);
        if(Serialize(msg_, &m_buffer[0], m_buffer.size()))
        {
          return(CPublisher::Send(&m_buffer[0], size, time_));
        }
      }
      else
      {
        // see !IsSubscribed()
        return(CPublisher::Send(nullptr, 0));
      }
      return(0);
    }

  private:
    virtual std::string GetTypeName() const = 0;
    virtual std::string GetDescription() const = 0;
    virtual size_t GetSize(const T& msg_) const = 0;
    virtual bool Serialize(const T& msg_, char* buffer_, size_t size_) const = 0;

    std::vector<char> m_buffer;
  };
}