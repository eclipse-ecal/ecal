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
      : m_subscriber(
        topic_name_,
        []() 
        {
          Deserializer deserializer;
          return deserializer.GetDataTypeInformation();
        }(),
        config_)
    {
    }

    CMessageSubscriber(const std::string& topic_name_, const SubEventCallbackT& event_callback_, const Subscriber::Configuration& config_ = GetSubscriberConfiguration())
      : m_subscriber(
        topic_name_,
        []()
        {
          Deserializer deserializer;
          return deserializer.GetDataTypeInformation();
        }(),
          event_callback_,
          config_)
    {
    }

    ~CMessageSubscriber() = default;

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
    CMessageSubscriber(CMessageSubscriber&& rhs) = default;

    /**
    * @brief  Move assignment
    **/
    CMessageSubscriber& operator=(CMessageSubscriber&& rhs) = default;

    /**
     * @brief Add a data callback callback for incoming messages.
     *
     * @param data_callback_   The callback function.
     * @param error_callback_  Error callback function, which is called in case the deserialization fails
     *
     * @return  True if it succeeds, false if it fails.
    **/
    void SetReceiveCallback(DataCallbackT data_callback_, DeserializationErrorCallbackT error_callback_ = nullptr)
    {
      std::shared_ptr<Deserializer> deserializer = std::make_shared<Deserializer>();
      /*
      * This is the internal Message Callback.
      * In case that any callbacks are registered, it tries to deserialize the data.
      * Serializers need to throw a DeserializationException in case that the deserialization of the data fails for any reason.
      * If the serialization does not throw, the data callback is invoked.
      * In case that it does fail, the error callback is invoked with the original information, so that the user may anaylze why the deserialization failed, if they are interested.
      */
      auto internal_receive_callback = [deserializer, data_callback_, error_callback_](const STopicId& publisher_id_, const SDataTypeInformation& data_type_info_, const SReceiveCallbackData& data_)
      {
        if (!data_callback_ && !error_callback_)
        {
          return;
        }

        try
        {
          auto msg = deserializer->Deserialize(data_.buffer, data_.buffer_size, data_type_info_);
          if (data_callback_)
          {
            data_callback_(publisher_id_, msg, data_.send_timestamp, data_.send_clock);
          }
        }
        catch (const DeserializationException& error)
        {
          if (error_callback_)
          {
            error_callback_(error.what(), publisher_id_, data_type_info_, data_);
          }
        }
      };

      m_subscriber.SetReceiveCallback(std::move(internal_receive_callback));
    }

    /**
     * @brief  Remove receive callback for incoming messages.
     *
     * @return  True if it succeeds, false if it fails.
    **/
    void RemoveReceiveCallback()
    {
      m_subscriber.RemoveReceiveCallback();
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

    CSubscriber m_subscriber;
  };
}
