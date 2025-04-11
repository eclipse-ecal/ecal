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
   * @brief eCAL google::protobuf publisher class.
   *
   * Publisher template  class for google::protobuf messages. For details see documentation of CPublisher class.
   *
  **/
  template <typename T, typename Serializer>
  class CMessagePublisher
  {
    class CPayload : public eCAL::CPayloadWriter
    {
    public:
      explicit CPayload(const T& message_, Serializer& serializer_) :
        message(message_),
        serializer(serializer_)
      {};

      ~CPayload() override = default;

      CPayload(const CPayload&) = default;
      CPayload(CPayload&&) noexcept = default;

      CPayload& operator=(const CPayload&) = delete;
      CPayload& operator=(CPayload&&) noexcept = delete;

      bool WriteFull(void* buf_, size_t len_) override
      {
        return serializer.Serialize(message, buf_, len_);
      }

      size_t GetSize() override {
        return serializer.MessageSize(message);
      };

    private:
      const T& message;
      Serializer& serializer;
    };

  public:
    /**
     * @brief  Constructor.
     *
     * @param topic_name_  Unique topic name.
     * @param config_      Optional configuration parameters.
    **/
    explicit CMessagePublisher(const std::string& topic_name_, const eCAL::Publisher::Configuration& config_ = GetPublisherConfiguration())
      : m_serializer{}
      , m_publisher(topic_name_, m_serializer.GetDataTypeInformation(), config_)
    {
    }

    /**
     * @brief Constructor.
     *
     * @param topic_name_      Unique topic name.
     * @param data_type_info_  Topic data type information (encoding, type, descriptor).
     * @param event_callback_  The publisher event callback funtion.
     * @param config_          Optional configuration parameters.
    **/
    explicit CMessagePublisher(const std::string& topic_name_, const PubEventCallbackT& event_callback_, const Publisher::Configuration& config_ = GetPublisherConfiguration())
      : m_serializer{}
      , m_publisher(topic_name_, m_serializer.GetDataTypeInformation(), event_callback_, config_)
    {
    }

    /**
     * @brief  Copy Constructor is not available.
    **/
    CMessagePublisher(const CMessagePublisher&) = delete;

    /**
     * @brief  Move Constructor
    **/
    CMessagePublisher(CMessagePublisher&&) = default;

    /**
     * @brief  Destructor.
    **/
    ~CMessagePublisher() = default;

    /**
     * @brief  Copy assignment is not available.
    **/
    CMessagePublisher& operator=(const CMessagePublisher&) = delete;

    /**
     * @brief  Move assignment
    **/
    CMessagePublisher& operator=(CMessagePublisher&&) = default;

    /**
     * @brief Send a serialized message to all subscribers.
     *
     * @param msg_                     The message object.
     * @param time_                    Time stamp.
     *
     * @return True if succeeded, otherwise false.
    **/
    bool Send(const T& msg_, long long time_ = CPublisher::DEFAULT_TIME_ARGUMENT)
    {
      CPayload payload{ msg_, m_serializer };
      return m_publisher.Send(payload, time_);
    }

    /**
     * @brief Query the number of subscribers.
     *
     * @return  Number of subscribers.
    **/
    size_t GetSubscriberCount() const
    {
      return m_publisher.GetSubscriberCount();
    };

    /**
     * @brief Retrieve the topic name.
     *
     * @return  The topic name.
    **/
    const std::string& GetTopicName() const
    {
      return m_publisher.GetTopicName();
    };

    /**
     * @brief Retrieve the topic id.
     *
     * @return  The topic id.
    **/
    const STopicId& GetTopicId() const
    {
      return m_publisher.GetTopicId();
    };

    /**
     * @brief Gets description of the connected topic.
     *
     * @return  The topic information.
    **/
    const SDataTypeInformation& GetDataTypeInformation() const
    {
      return m_publisher.GetDataTypeInformation();
    };


  private:
    Serializer         m_serializer;
    CPublisher         m_publisher;
  };
}
