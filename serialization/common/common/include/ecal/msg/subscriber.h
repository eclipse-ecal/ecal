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
#include <ecal/msg/exception.h>
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
     * @brief eCAL message receive callback function.
     *
     * @param topic_id_    Unique topic id of the publisher who published the data
     * @param msg_         Message content.
     * @param time_        Message time stamp.
     * @param clock_       Message writer clock.
     **/
    using DataCallbackT = std::function<void(const STopicId& publisher_id_, const T& msg_, long long time_, long long clock_)>;

    /**
     * @brief Error callback that can be called if the deserialization of the incoming data has failed.
     *
     * @param error_message_ Error
     * @param topic_id_      Unique topic id of the publisher who published the data
     * @param msg_           Message content.
     * @param time_          Message time stamp.
     * @param clock_         Message writer clock.
    **/
    using DeserializationErrorCallbackT = std::function<void(const std::string& error_message_, const STopicId& publisher_id_, const SDataTypeInformation& data_type_info_, const SReceiveCallbackData& data_)>;

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
      SetInternalReceiveCallback();
    }

    CMessageSubscriber(const std::string& topic_name_, const SubEventCallbackT& event_callback_, const Subscriber::Configuration& config_ = GetSubscriberConfiguration())
      : m_deserializer()
      , m_subscriber(topic_name_, m_deserializer.GetDataTypeInformation(), event_callback_, config_)
    {
      SetInternalReceiveCallback();
    }

    ~CMessageSubscriber()
    {
      RemoveReceiveCallback();
      RemoveErrorCallback();

      RemoveInternalReceiveCallback();
    }

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
    {
      // By first removing the old receive callbackand adding it back later, we might potentially loose a few messages during that time
      // If they are received in between removing the old callback and putting in the new one.
      // However that's better than risking data races???

      rhs.RemoveInternalReceiveCallback();

      std::lock_guard<std::mutex> callback_lock(rhs.m_callback_mutex);
      m_data_callback = std::move(rhs.m_data_callback);
      m_error_callback = std::move(rhs.m_error_callback);
      
      SetInternalReceiveCallback();
    }

    /**
    * @brief  Move assignment
    **/
    CMessageSubscriber& operator=(CMessageSubscriber&& rhs) = delete;

    /**
     * @brief Add a data callback callback for incoming messages.
     *
     * @param callback_  The callback function.
     *
     * @return  True if it succeeds, false if it fails.
    **/
    void SetReceiveCallback(DataCallbackT callback_)
    {
      std::lock_guard<std::mutex> callback_lock(m_callback_mutex);
      m_data_callback = callback_;
    }

    /**
     * @brief  Remove receive callback for incoming messages.
     *
     * @return  True if it succeeds, false if it fails.
    **/
    void RemoveReceiveCallback()
    {
      std::lock_guard<std::mutex> callback_lock(m_callback_mutex);
      m_data_callback = nullptr;
    }

    /**
     * @brief Set Error callback which is called if the deserialization of the incoming data fails.
     *
     * @param callback_  The callback function.
     *
     * @return  True if it succeeds, false if it fails.
    **/
    void SetErrorCallback(DeserializationErrorCallbackT callback_)
    {
      std::lock_guard<std::mutex> callback_lock(m_callback_mutex);
      m_error_callback = callback_;
    }

    /**
     * @brief  Remove receive callback for incoming messages.
     *
     * @return  True if it succeeds, false if it fails.
    **/
    void RemoveErrorCallback()
    {
      std::lock_guard<std::mutex> callback_lock(m_callback_mutex);
      m_error_callback = nullptr;
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
    const std::string& GetTopicName() const
    {
      return m_subscriber.GetTopicName();
    }

    /**
     * @brief Retrieve the topic id.
     *
     * @return  The topic id.
    **/
    const STopicId& GetTopicId() const
    {
      return m_subscriber.GetTopicId();
    }

    /**
     * @brief Retrieve the topic information.
     *
     * @return  The topic information.
    **/
    const SDataTypeInformation& GetDataTypeInformation() const
    {
      return m_subscriber.GetDataTypeInformation();
    }


  private:
    /*
    * This is the internal Message Callback. 
    * In case that any callbacks are registered, it tries to deserialize the data.
    * Serializers need to throw a DeserializationException in case that the deserialization of the data fails for any reason.
    * If the serialization does not throw, the data callback is invoked.
    * In case that it does fail, the error callback is invoked with the original information, so that the user may anaylze why the deserialization failed, if they are interested.
    */
    void InternalReceiveCallback(const STopicId& publisher_id_, const SDataTypeInformation& data_type_info_, const SReceiveCallbackData& data_)
    {
      {
        std::lock_guard<std::mutex> callback_lock(m_callback_mutex);
        
        if (!m_data_callback && !m_error_callback)
        {
          return;
        }

        try
        {
          auto msg = m_deserializer.Deserialize(data_.buffer, data_.buffer_size, data_type_info_);
          if (m_data_callback)
          {
            m_data_callback(publisher_id_, msg, data_.send_timestamp, data_.send_clock);
          }
        }
        catch (const DeserializationException& error)
        {
          if (m_error_callback)
          {
            m_error_callback(error.what(), publisher_id_, data_type_info_, data_);
          }
        }
      }
    }

    void SetInternalReceiveCallback()
    {
      auto internal_callback = [this](const STopicId& publisher_id_, const SDataTypeInformation& data_type_info_, const SReceiveCallbackData& data_)
      {
        this->InternalReceiveCallback(publisher_id_, data_type_info_, data_);
      };
      m_subscriber.SetReceiveCallback(internal_callback);
    }

    void RemoveInternalReceiveCallback()
    {
      m_subscriber.RemoveReceiveCallback();
    }

    Deserializer                   m_deserializer;
    CSubscriber                    m_subscriber;

    std::mutex                     m_callback_mutex; // we need only one mutex to protect both callbacks
    DataCallbackT                  m_data_callback;
    DeserializationErrorCallbackT  m_error_callback;
  };
}
