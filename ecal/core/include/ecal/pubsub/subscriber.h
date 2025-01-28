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
 * @file   pubsub/subscriber.h
 * @brief  eCAL subscriber interface
**/

#pragma once

#include <ecal/deprecate.h>
#include <ecal/namespace.h>
#include <ecal/os.h>

#include <ecal/pubsub/types.h>
#include <ecal/config.h>

#include <memory>
#include <string>

namespace eCAL
{
  class CSubscriberImpl;

  /**
   * @brief eCAL subscriber class.
  **/
  class ECAL_API_CLASS CSubscriber
  {
  public:
    /**
     * @brief Constructor.
     *
     * @param topic_name_      Unique topic name.
     * @param data_type_info_  Topic data type information (encoding, type, descriptor).
     * @param config_          Optional configuration parameters.
    **/
    ECAL_API_EXPORTED_MEMBER
      CSubscriber(const std::string& topic_name_, const SDataTypeInformation& data_type_info_ = SDataTypeInformation(), const Subscriber::Configuration& config_ = GetSubscriberConfiguration());

    /**
     * @brief Constructor with event callback registration.
     *
     * @param topic_name_      Unique topic name.
     * @param data_type_info_  Topic data type information (encoding, type, descriptor).
     * @param event_callback_  Callback for subscriber events.
     * @param config_          Configuration parameters.
    **/
    ECAL_API_EXPORTED_MEMBER
      CSubscriber(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const SubEventCallbackT& event_callback_, const Subscriber::Configuration& config_ = GetSubscriberConfiguration());

    /**
     * @brief Destructor.
    **/
    ECAL_API_EXPORTED_MEMBER
      virtual ~CSubscriber();

    /**
     * @brief CSubscribers are non-copyable.
    **/
    CSubscriber(const CSubscriber&) = delete;

    /**
     * @brief CSubscribers are non-copyable.
    **/
    CSubscriber& operator=(const CSubscriber&) = delete;

    /**
     * @brief CSubscribers are move-enabled.
    **/
    ECAL_API_EXPORTED_MEMBER
      CSubscriber(CSubscriber&& rhs) noexcept;

    /**
     * @brief CSubscribers are move-enabled.
    **/
    ECAL_API_EXPORTED_MEMBER
      CSubscriber& operator=(CSubscriber&& rhs) noexcept;

    /**
     * @brief Set/overwrite callback function for incoming receives.
     *
     * @param callback_  The callback function to set.
     *
     * @return  True if succeeded, false if not.
    **/
    ECAL_API_EXPORTED_MEMBER
      bool SetReceiveCallback(ReceiveCallbackT callback_);

    /**
     * @brief Remove callback function for incoming receives.
     *
     * @return  True if succeeded, false if not.
    **/
    ECAL_API_EXPORTED_MEMBER
      bool RemoveReceiveCallback();

    /**
     * @brief Query the number of connected publishers.
     *
     * @return  Number of publishers.
    **/
    ECAL_API_EXPORTED_MEMBER
      size_t GetPublisherCount() const;

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
     * @brief Retrieve the topic information.
     *
     * @return  The topic information.
    **/
    ECAL_API_EXPORTED_MEMBER
      SDataTypeInformation GetDataTypeInformation() const;

  private:
    std::shared_ptr<CSubscriberImpl> m_subscriber_impl;
  };
}
