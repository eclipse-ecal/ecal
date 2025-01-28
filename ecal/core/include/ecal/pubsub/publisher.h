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
 * @file   pubsub/publisher.h
 * @brief  eCAL publisher interface
**/

#pragma once

#include <ecal/deprecate.h>
#include <ecal/namespace.h>
#include <ecal/os.h>

#include <ecal/pubsub/types.h>
#include <ecal/pubsub/payload_writer.h>

#include <ecal/config.h>

#include <memory>
#include <string>

namespace eCAL
{
  class CPublisherImpl;

  /**
   * @brief eCAL publisher class.
  **/
  class ECAL_API_CLASS CPublisher
  {
  public:
    ECAL_API_EXPORTED_MEMBER
      static constexpr long long DEFAULT_TIME_ARGUMENT = -1;  /*!< Use DEFAULT_TIME_ARGUMENT in the `Send()` function to let eCAL determine the send timestamp */

    /**
     * @brief Constructor.
     *
     * @param topic_name_      Unique topic name.
     * @param data_type_info_  Topic data type information (encoding, type, descriptor).
     * @param config_          Optional configuration parameters.
    **/
    ECAL_API_EXPORTED_MEMBER
      CPublisher(const std::string& topic_name_, const SDataTypeInformation& data_type_info_ = SDataTypeInformation(), const Publisher::Configuration& config_ = GetPublisherConfiguration());

    /**
     * @brief Constructor.
     *
     * @param topic_name_      Unique topic name.
     * @param data_type_info_  Topic data type information (encoding, type, descriptor).
     * @param event_callback_  The publisher event callback funtion.
     * @param config_          Optional configuration parameters.
    **/
    ECAL_API_EXPORTED_MEMBER
      CPublisher(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const PubEventCallbackT& event_callback_, const Publisher::Configuration& config_ = GetPublisherConfiguration());

    /**
     * @brief Destructor.
    **/
    ECAL_API_EXPORTED_MEMBER
      virtual ~CPublisher();

    /**
     * @brief CPublishers are non-copyable
    **/
    CPublisher(const CPublisher&) = delete;

    /**
     * @brief CPublishers are non-copyable
    **/
    CPublisher& operator=(const CPublisher&) = delete;

    /**
     * @brief CPublishers are move-enabled
    **/
    ECAL_API_EXPORTED_MEMBER
      CPublisher(CPublisher&& rhs) noexcept;

    /**
     * @brief CPublishers are move-enabled
    **/
    ECAL_API_EXPORTED_MEMBER
      CPublisher& operator=(CPublisher&& rhs) noexcept;

    /**
     * @brief Send a message to all subscribers.
     *
     * @param buf_    Pointer to content buffer.
     * @param len_    Length of buffer.
     * @param time_   Send time (-1 = use eCAL system time in us, default = -1).
     *
     * @return  True if succeeded, false if not.
    **/
    ECAL_API_EXPORTED_MEMBER
      bool Send(const void* buf_, size_t len_, long long time_ = DEFAULT_TIME_ARGUMENT);

    /**
     * @brief Send a message to all subscribers.
     *
     * @param payload_  Payload writer.
     * @param time_     Send time (-1 = use eCAL system time in us, default = -1).
     *
     * @return  True if succeeded, false if not.
    **/
    ECAL_API_EXPORTED_MEMBER
      bool Send(CPayloadWriter& payload_, long long time_ = DEFAULT_TIME_ARGUMENT);

    /**
     * @brief Send a message to all subscribers.
     *
     * @param payload_  Payload string.
     * @param time_     Send time (-1 = use eCAL system time in us, default = -1).
     *
     * @return  True if succeeded, false if not.
    **/
    ECAL_API_EXPORTED_MEMBER
      bool Send(const std::string& payload_, long long time_ = DEFAULT_TIME_ARGUMENT);

    /**
     * @brief Query the number of subscribers.
     *
     * @return  Number of subscribers.
    **/
    ECAL_API_EXPORTED_MEMBER
      size_t GetSubscriberCount() const;

    /**
     * @brief Retrieve the topic name.
     *
     * @return  The topic name.
    **/
    ECAL_API_EXPORTED_MEMBER
      std::string GetTopicName() const;

    /**
     * @brief Retrieve the topic id.
     *
     * @return  The topic id.
    **/
    ECAL_API_EXPORTED_MEMBER
      STopicId GetTopicId() const;

    /**
     * @brief Gets description of the connected topic.
     *
     * @return  The topic information.
    **/
    ECAL_API_EXPORTED_MEMBER
      SDataTypeInformation GetDataTypeInformation() const;

  private:
    std::shared_ptr<CPublisherImpl> m_publisher_impl;
  };

}
