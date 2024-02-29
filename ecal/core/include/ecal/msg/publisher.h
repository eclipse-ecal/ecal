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

#include <ecal/ecal_deprecate.h>
#include <ecal/ecal_publisher.h>
#include <ecal/ecal_util.h>

#include <string>
#include <vector>
#include <functional>
#include <cassert>
#include <cstring>

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
     * @brief  Default Constructor. 
     *         Using this constructor, the object is not actually in a usable state.
     *         Before being able to send data, one has to call the `Create()` function, first.
    **/
    CMsgPublisher() : CPublisher()
    {
    }

    /**
     * @brief  Constructor, that automatically intializes the Publisher. 
     *         This should be the preferred constructor.
     *
     * @param topic_name_  Unique topic name.
     * @param topic_info_  Struct that contains information of the datatype (name, encoding, description) of the topic.
    **/
    CMsgPublisher(const std::string& topic_name_, const struct SDataTypeInformation& topic_info_) : CPublisher(topic_name_, topic_info_)
    {
    }

    /**
     * @brief  Constructor, that automatically intializes the Publisher. 
     *         If no datatype information about the topic is available, this constructor can be used.
     *
     * @param topic_name_  Unique topic name.
    **/
    explicit CMsgPublisher(const std::string& topic_name_) : CMsgPublisher(topic_name_, GetDataTypeInformation())
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

    ~CMsgPublisher() override = default;

    /**
     * @brief  Creates this object.
     *
     * @param topic_name_   Unique topic name.
     * @param topic_info_   Associated datatype description.
     *
     * @return  True if it succeeds, false if it fails.
    **/
    bool Create(const std::string& topic_name_, const struct SDataTypeInformation& topic_info_)
    {
      return(CPublisher::Create(topic_name_, topic_info_));
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
    size_t Send(const T& msg_, long long time_ = DEFAULT_TIME_ARGUMENT)
    {
      // this is an optimization ...
      // if there is no subscription we do not waste time for
      // serialization, but we send an empty payload
      // to still do some statistics like message clock
        // counting and frequency calculation for the monitoring layer
      if (!IsSubscribed())
      {
        return(CPublisher::Send(nullptr, 0, time_));
      }

      // if we have a subscription allocate memory for the
      // binary stream, serialize the message into the
      // buffer and finally send it with a binary publisher
      size_t size = GetSize(msg_);
      if (size > 0)
      {
        m_buffer.resize(size);
        if (Serialize(msg_, m_buffer.data(), m_buffer.size()))
        {
          return(CPublisher::Send(m_buffer.data(), size, time_));
        }
      }
      else
      {
        // send a zero payload length message to trigger the subscriber side
        return(CPublisher::Send(nullptr, 0, time_));
      }
      return(0);
    }

  protected:
    // We cannot make it pure virtual, as it would break a bunch of implementations, who are not (yet) implementing this function
    virtual struct SDataTypeInformation GetDataTypeInformation() const { return SDataTypeInformation{}; }
  private:
    virtual size_t GetSize(const T& msg_) const = 0;
    virtual bool Serialize(const T& msg_, char* buffer_, size_t size_) const = 0;

    std::vector<char> m_buffer;
  };
}
