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
 * @file   msg/subscriber.h
 * @brief  eCAL message subscriber interface
**/

#pragma once

#include <ecal/deprecate.h>
#include <ecal/pubsub/subscriber.h>
#include <ecal/util.h>

#include <cassert>
#include <cstring>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

namespace eCAL
{
  /**
   * @brief  eCAL abstract message subscriber class.
   *
   * Abstract subscriber template class for messages.
   * This class has two template arguments, the actual type and a deserializer class.
   * The deserializer class is responsible for providing datatype information, and providing a method to convert from `void*` to `T`.
   * This allows to specify classes with common deserializers, e.g. like a ProtobufMessageSubscriber, StringMessageSubscriber etc.
   *
  **/
  template <typename T, typename Deserializer>
  class CMessageSubscriber
  {
  public:
    /**
    * @brief  Constructor.
    *
    * @param topic_name_  Unique topic name.
    * @param config_      Optional configuration parameters.
    **/
    explicit CMessageSubscriber(const std::string& topic_name_, const Subscriber::Configuration& config_ = GetSubscriberConfiguration()) 
      : m_deserializer()
      , m_subscriber(topic_name_, m_deserializer.GetDataTypeInformation(), config_)
    {
    }

    explicit CMessageSubscriber(const std::string& topic_name_, const SubEventCallbackT& event_callback_, const Subscriber::Configuration& config_ = GetSubscriberConfiguration())
      : m_deserializer()
      , m_subscriber(topic_name_, m_deserializer.GetDataTypeInformation(), event_callback_, config_)
    {
    }

    ~CMessageSubscriber() noexcept = default;

    /**
    * @brief  Copy Constructor is not available.
    **/
    CMessageSubscriber(const CMessageSubscriber&) = delete;

    /**
    * @brief  Copy Constructor is not available.
    **/
    CMessageSubscriber& operator=(const CMessageSubscriber&) = delete;

    /**
    * @brief  Move Constructor
    **/
    CMessageSubscriber(CMessageSubscriber&& rhs)
      : m_deserializer(std::move(rhs.m_deserializer))
      , m_subscriber(std::move(rhs.m_subscriber))
      , m_cb_callback(std::move(rhs.m_cb_callback))
    {
      bool has_callback = (m_cb_callback != nullptr);

      if (has_callback)
      {
        // the callback bound to the CSubscriber belongs to rhs, bind to this callback instead
        m_subscriber.RemoveReceiveCallback();
        auto callback = std::bind(&CMessageSubscriber::ReceiveCallback, this, std::placeholders::_1, std::placeholders::_3);
        m_subscriber.SetReceiveCallback(callback);
      }
    }

    /**
    * @brief  Move assignment
    **/
    CMessageSubscriber& operator=(CMessageSubscriber&& rhs) = delete;

    /**
     * @brief eCAL message receive callback function
     *
     * @param topic_id_    Unique topic id.
     * @param msg_         Message content.
     * @param time_        Message time stamp.
     * @param clock_       Message writer clock.
     **/
    using MsgReceiveCallbackT = std::function<void(const STopicId& topic_id_, const T& msg_, long long time_, long long clock_)>;

    /**
     * @brief  Add receive callback for incoming messages.
     *
     * @param callback_  The callback function.
     *
     * @return  True if it succeeds, false if it fails.
    **/
    bool SetReceiveCallback(MsgReceiveCallbackT callback_)
    {
      RemoveReceiveCallback();
      {
        std::lock_guard<std::mutex> callback_lock(m_cb_callback_mutex);
        m_cb_callback = callback_;
      }
      auto callback = std::bind(&CMessageSubscriber::ReceiveCallback, this, std::placeholders::_1, std::placeholders::_3);
      return(m_subscriber.SetReceiveCallback(callback));
    }

    /**
     * @brief  Remove receive callback for incoming messages.
     *
     * @return  True if it succeeds, false if it fails.
    **/
    bool RemoveReceiveCallback()
    {
      bool ret = m_subscriber.RemoveReceiveCallback();

      std::lock_guard<std::mutex> callback_lock(m_cb_callback_mutex);
      if (m_cb_callback == nullptr) return(false);
      m_cb_callback = nullptr;
      return(ret);
    }

    /**
     * @brief Query the number of connected publishers.
     *
     * @return  Number of publishers.
    **/
    size_t GetPublisherCount() const
    {
      return m_subscriber.GetPublisherCount();
    }

    /**
     * @brief Retrieve the topic name.
     *
     * @return  The topic name.
    **/
    std::string GetTopicName() const
    {
      return m_subscriber.GetTopicName();
    }

    /**
     * @brief Retrieve the topic id.
     *
     * @return  The topic id.
    **/
    STopicId GetTopicId() const
    {
      return m_subscriber.GetTopicId();
    }

    /**
     * @brief Retrieve the topic information.
     *
     * @return  The topic information.
    **/
    SDataTypeInformation GetDataTypeInformation() const
    {
      return m_subscriber.GetDataTypeInformation();
    }


  private:
    void ReceiveCallback(const STopicId& topic_id_, const SReceiveCallbackData& data_)
    {
      MsgReceiveCallbackT fn_callback = nullptr;
      {
        std::lock_guard<std::mutex> callback_lock(m_cb_callback_mutex);
        fn_callback = m_cb_callback;
      }

      if (fn_callback == nullptr) return;

      T msg{};
      if (m_deserializer.Deserialize(msg, data_.buffer, data_.buffer_size))
      {
        (fn_callback)(topic_id_, msg, data_.send_timestamp, data_.send_clock);
      }
      // TODO: Here we should handle error callbacks
    }

    Deserializer         m_deserializer;
    CSubscriber          m_subscriber;

    std::mutex           m_cb_callback_mutex;
    MsgReceiveCallbackT  m_cb_callback;
  };
}
