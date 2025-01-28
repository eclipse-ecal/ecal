/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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
 * @file   msg/publisher.h
 * @brief  eCAL message publisher interface
**/

#pragma once

#include <ecal/deprecate.h>
#include <ecal/pubsub/publisher.h>
#include <ecal/util.h>

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
     * @brief  Constructor.
     *
     * @param topic_name_      Unique topic name.
     * @param data_type_info_  Topic data type information (encoding, type, descriptor).
     * @param config_          Optional configuration parameters.
    **/
    explicit CMsgPublisher(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const Publisher::Configuration& config_ = GetPublisherConfiguration()) : CPublisher(topic_name_, data_type_info_, config_)
    {
    }

    /**
     * @brief  Constructor.
     *
     * @param topic_name_      Unique topic name.
     * @param data_type_info_  Topic data type information (encoding, type, descriptor).
     * @param event_callback_  The publisher event callback funtion.
     * @param config_          Optional configuration parameters.
    **/
    explicit CMsgPublisher(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const PubEventCallbackT& event_callback_, const Publisher::Configuration& config_ = GetPublisherConfiguration()) : CPublisher(topic_name_, data_type_info_, event_callback_, config_)
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
     * @brief  Send serialized message. 
     *
     * @param msg_   The message object. 
     * @param time_  Optional time stamp. 
     *
     * @return  True if succeeded, false if not.
    **/
    bool Send(const T& msg_, long long time_ = DEFAULT_TIME_ARGUMENT)
    {
      // this is an optimization ...
      // if there is no subscription we do not waste time for
      // serialization, but we send an empty payload
      // to still do some statistics like message clock
        // counting and frequency calculation for the monitoring layer
      if (CPublisher::GetSubscriberCount() == 0)
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
      return false;
    }

  protected:
    // We cannot make it pure virtual, as it would break a bunch of implementations, who are not (yet) implementing this function
    virtual SDataTypeInformation GetDataTypeInformation() const { return SDataTypeInformation{}; }
  private:
    virtual size_t GetSize(const T& msg_) const = 0;
    virtual bool Serialize(const T& msg_, char* buffer_, size_t size_) const = 0;

    std::vector<char> m_buffer;
  };
}
