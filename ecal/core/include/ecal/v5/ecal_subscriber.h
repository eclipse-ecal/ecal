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
 * @file   v5/ecal_subscriber.h
 * @brief  eCAL subscriber interface (deprecated eCAL5 version)
**/

#pragma once

#include <ecal/namespace.h>

#include <cstddef>
#include <ecal/v5/ecal_callback.h>
#include <ecal/deprecate.h>
#include <ecal/os.h>
#include <ecal/config.h>
#include <ecal/types.h>

#include <memory>
#include <set>
#include <string>

namespace eCAL
{
  class CSubscriberImpl;

  ECAL_CORE_NAMESPACE_V5
  {
    /**
     * @brief eCAL subscriber class.
    **/

    class ECAL_API_CLASS CSubscriber
    {
    public:
      /**
       * @brief Constructor.
      **/
      ECAL_API_EXPORTED_MEMBER
        CSubscriber();

      /**
       * @brief Constructor.
       *
       * @param topic_name_      Unique topic name.
       * @param data_type_info_  Topic data type information (encoding, type, descriptor).
       * @param config_          Optional configuration parameters.
      **/
      ECAL_API_EXPORTED_MEMBER
        CSubscriber(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const Subscriber::Configuration& config_ = GetSubscriberConfiguration());

      /**
       * @brief Constructor.
       *
       * @param topic_name_      Unique topic name.
       * @param config_          Optional configuration parameters.
      **/
      ECAL_API_EXPORTED_MEMBER
        explicit CSubscriber(const std::string& topic_name_, const Subscriber::Configuration& config_ = GetSubscriberConfiguration());

      /**
       * @brief Destructor.
      **/
      ECAL_API_EXPORTED_MEMBER
        virtual ~CSubscriber();

      /**
       * @brief CSubscribers are non-copyable
      **/
      CSubscriber(const CSubscriber&) = delete;

      /**
       * @brief CSubscribers are non-copyable
      **/
      CSubscriber& operator=(const CSubscriber&) = delete;

      /**
       * @brief CSubscribers are move-enabled
      **/
      ECAL_API_EXPORTED_MEMBER
        CSubscriber(CSubscriber&& rhs) noexcept;

      /**
       * @brief CSubscribers are move-enabled
      **/
      ECAL_API_EXPORTED_MEMBER
        CSubscriber& operator=(CSubscriber&& rhs) noexcept;

      /**
       * @brief Creates this object.
       *
       * @param topic_name_      Unique topic name.
       * @param data_type_info_  Topic data type information (encoding, type, descriptor).
       * @param config_          Optional configuration parameters.
       *
       * @return  True if it succeeds, false if it fails.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool Create(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const Subscriber::Configuration& config_ = GetSubscriberConfiguration());

      /**
       * @brief Creates this object.
       *
       * @param topic_name_   Unique topic name.
       *
       * @return  True if it succeeds, false if it fails.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool Create(const std::string& topic_name_);

      /**
       * @brief Destroys this object.
       *
       * @return  true if it succeeds, false if it fails.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool Destroy();

      /**
       * @brief Set a set of id's to prefiltering topics (see CPublisher::SetID).
       *
       * @param filter_ids_  Set of filter id's.
       *
       * @return  True if it succeeds, false if it fails.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool SetID(const std::set<long long>& filter_ids_);

      /**
       * @brief Sets subscriber attribute.
       *
       * @param attr_name_   Attribute name.
       * @param attr_value_  Attribute value.
       *
       * @return  True if it succeeds, false if it fails.
       * @experimental
      **/
      ECAL_API_EXPORTED_MEMBER
        bool SetAttribute(const std::string& attr_name_, const std::string& attr_value_);

      /**
       * @brief Removes subscriber attribute.
       *
       * @param attr_name_   Attribute name.
       *
       * @return  True if it succeeds, false if it fails.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool ClearAttribute(const std::string& attr_name_);

      /**
       * @brief Receive a message from the publisher (able to process zero length buffer).
       *
       * @param [out] buf_    Standard string for copying message content.
       * @param [out] time_   Time from publisher in us (default = nullptr).
       * @param rcv_timeout_  Maximum time before receive operation returns (in milliseconds, -1 means infinite).
       *
       * @return  True if it succeeds, false if it fails.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool ReceiveBuffer(std::string& buf_, long long* time_ = nullptr, int rcv_timeout_ = 0) const;

      /**
       * @brief Add callback function for incoming receives.
       *
       * @param callback_  The callback function to add.
       *
       * @return  True if succeeded, false if not.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool AddReceiveCallback(ReceiveCallbackT callback_);

      /**
       * @brief Add callback function for incoming receives.
       *
       * @param callback_  The callback function to add.
        *
        * @return  True if succeeded, false if not.
       **/
      ECAL_API_EXPORTED_MEMBER
        bool AddReceiveCallback(eCAL::ReceiveCallbackT callback_);

      /**
       * @brief Remove callback function for incoming receives.
       *
       * @return  True if succeeded, false if not.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool RemReceiveCallback();

      /**
       * @brief Add callback function for subscriber events.
       *
       * @param type_      The event type to react on.
       * @param callback_  The callback function to add.
       *
       * @return  True if succeeded, false if not.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool AddEventCallback(eSubscriberEvent type_, SubEventCallbackT callback_);

      /**
       * @brief Remove callback function for subscriber events.
       *
       * @param type_  The event type to remove.
       *
       * @return  True if succeeded, false if not.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool RemEventCallback(eSubscriberEvent type_);

      /**
       * @brief Query if this object is created.
       *
       * @return  true if created, false if not.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool IsCreated() const { return(m_subscriber_impl != nullptr); }

      /**
       * @brief Query if the subscriber is published.
       *
       * @return  true if published, false if not.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool IsPublished() const;

      /**
       * @brief Query the number of publishers.
       *
       * @return  Number of publishers.
      **/
      ECAL_API_EXPORTED_MEMBER
        size_t GetPublisherCount() const;

      /**
       * @brief Gets name of the connected topic.
       *
       * @return  The topic name.
      **/
      ECAL_API_EXPORTED_MEMBER
        std::string GetTopicName() const;

      /**
       * @brief Gets a unique ID of this Subscriber
       *
       * @return  The topic id.
      **/
      ECAL_API_EXPORTED_MEMBER
        STopicId GetId() const;

      /**
       * @brief Gets description of the connected topic.
       *
       * @return  The topic information.
      **/
      ECAL_API_EXPORTED_MEMBER
        SDataTypeInformation GetDataTypeInformation() const;

      /**
       * @brief Dump the whole class state into a string.
       *
       * @param indent_  Indentation used for dump.
       *
       * @return  The dump sting.
      **/
      ECAL_API_EXPORTED_MEMBER
        std::string Dump(const std::string& indent_ = "") const;

    private:
      // class members
      std::shared_ptr<CSubscriberImpl> m_subscriber_impl;
    };
  }
}
