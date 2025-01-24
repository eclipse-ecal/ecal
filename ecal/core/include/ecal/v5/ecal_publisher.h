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
 * @file   v5/ecal_publisher.h
 * @brief  eCAL publisher interface (deprecated eCAL5 version)
**/

#pragma once

#include <ecal/namespace.h>

#include <ecal/v5/ecal_callback.h>
#include <ecal/deprecate.h>
#include <ecal/os.h>
#include <ecal/pubsub/payload_writer.h>
#include <ecal/config.h>
#include <ecal/types.h>
#include <ecal/config/publisher.h>

#include <chrono>
#include <cstddef>
#include <memory>
#include <string>

namespace eCAL
{
  class CPublisherImpl;

  ECAL_CORE_NAMESPACE_V5
  {
    /**
     * @brief eCAL publisher class.
     *
     * The CPublisher class is used to send topics to matching eCAL subscribers. The topic is created automatically by the constructor
     * or by the Create member function.
     * <br>
     * <br>
     * For sending the topic payload the publisher class provides an overloaded Send method. The first one is sending the payload as
     * a std::string. The second needs a preallocated buffer described by a buffer address and a buffer length. The publisher is not
     * taking the ownership for the allocated memory buffer.
     * <br>
     * <br>
     * An optional time stamp can be attached to the topic payload.
     * 
    **/
    /**
     * @code
     *            // create publisher, topic name "A"
     *            eCAL::CPublisher pub("A");
     *
     *            // send string
     *            std::string send_s = "Hello World ";
     *
     *            // send content
     *            size_t snd_len = pub.Send(send_s);
     * @endcode
    **/
    class ECAL_API_CLASS CPublisher
    {
    public:
      ECAL_API_EXPORTED_MEMBER
        static constexpr long long DEFAULT_TIME_ARGUMENT = -1;  /*!< Use DEFAULT_TIME_ARGUMENT in the `Send()` function to let eCAL determine the send timestamp */

      /**
       * @brief Constructor. 
      **/
      ECAL_API_EXPORTED_MEMBER
        CPublisher();

      /**
       * @brief Constructor.
       *
       * @param topic_name_      Unique topic name.
       * @param data_type_info_  Topic data type information (encoding, type, descriptor).
       * @param config_          Optional configuration parameters.
      **/
      ECAL_API_EXPORTED_MEMBER
        CPublisher(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const Publisher::Configuration& config_ = GetPublisherConfiguration());

      /**
       * @brief Constructor.
       *
       * @param topic_name_   Unique topic name.
       * @param config_       Optional configuration parameters.
      **/
      ECAL_API_EXPORTED_MEMBER
        explicit CPublisher(const std::string& topic_name_, const Publisher::Configuration& config_ = GetPublisherConfiguration());

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
       * @brief Creates this object.
       *
       * @param topic_name_      Unique topic name.
       * @param data_type_info_  Topic data type information (encoding, type, descriptor).
       * @param config_          Optional configuration parameters.
       *
       * @return  True if it succeeds, false if it fails.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool Create(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const Publisher::Configuration& config_ = GetPublisherConfiguration());

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
       * @return  True if it succeeds, false if it fails. 
      **/
      ECAL_API_EXPORTED_MEMBER
        bool Destroy();

      /**
       * @brief Setup topic information.
       *
       * @param data_type_info_  Topic data type information attributes.
       *
       * @return  True if it succeeds, false if it fails.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool SetDataTypeInformation(const SDataTypeInformation& data_type_info_);

      /**
       * @brief Sets publisher attribute. 
       *
       * @param attr_name_   Attribute name. 
       * @param attr_value_  Attribute value. 
       *
       * @return  True if it succeeds, false if it fails. 
      **/
      ECAL_API_EXPORTED_MEMBER
        bool SetAttribute(const std::string& attr_name_, const std::string& attr_value_);

      /**
       * @brief Removes publisher attribute. 
       *
       * @param attr_name_   Attribute name. 
       *
       * @return  True if it succeeds, false if it fails.
       * @experimental
      **/
      ECAL_API_EXPORTED_MEMBER
        bool ClearAttribute(const std::string& attr_name_);

      /**
       * @brief Set the specific topic filter id.
       *
       * @param filter_id_  The topic id for subscriber side filtering (0 == no id).
       *
       * @return  True if it succeeds, false if it fails.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool SetID(long long filter_id_);

      /**
       * @brief Send a message to all subscribers. 
       *
       * @param buf_    Pointer to content buffer. 
       * @param len_    Length of buffer. 
       * @param time_   Send time (-1 = use eCAL system time in us, default = -1).
       *
       * @return  Number of bytes sent. 
      **/
      ECAL_API_EXPORTED_MEMBER
        size_t Send(const void* buf_, size_t len_, long long time_ = DEFAULT_TIME_ARGUMENT);

      /**
       * @brief Send a message to all subscribers.
       *
       * @param payload_  Payload.
       * @param time_     Send time (-1 = use eCAL system time in us, default = -1).
       *
       * @return  Number of bytes sent.
      **/
      ECAL_API_EXPORTED_MEMBER
        size_t Send(CPayloadWriter& payload_, long long time_ = DEFAULT_TIME_ARGUMENT);

      /**
       * @brief Send a message to all subscribers.
       *
       * @param s_      String that contains content to send.
       * @param time_   Send time (-1 = use eCAL system time in us, default = -1).
       *
       * @return  Number of bytes sent.
      **/
      ECAL_API_EXPORTED_MEMBER
        size_t Send(const std::string& s_, long long time_ = DEFAULT_TIME_ARGUMENT);

      /**
       * @brief Add callback function for publisher events.
       *
       * @param type_      The event type to react on.
       * @param callback_  The callback function to add.
       *
       * @return  True if succeeded, false if not.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool AddEventCallback(ePublisherEvent type_, PubEventCallbackT callback_);

      /**
       * @brief Remove callback function for publisher events.
       *
       * @param type_  The event type to remove.
       *
       * @return  True if succeeded, false if not.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool RemEventCallback(ePublisherEvent type_);

      /**
       * @brief Query if the publisher is created. 
       *
       * @return  True if created, false if not. 
      **/
      ECAL_API_EXPORTED_MEMBER
        bool IsCreated() const { return(m_publisher_impl != nullptr); }

      /**
       * @brief Query if the publisher is subscribed. 
       *
       * @return  true if subscribed, false if not. 
      **/
      ECAL_API_EXPORTED_MEMBER
        bool IsSubscribed() const;

      /**
       * @brief Query the number of subscribers.
       *
       * @return  Number of subscribers.
      **/
      ECAL_API_EXPORTED_MEMBER
        size_t GetSubscriberCount() const;

      /**
       * @brief Gets name of the connected topic. 
       *
       * @return  The topic name. 
      **/
      ECAL_API_EXPORTED_MEMBER
        std::string GetTopicName() const;

      /**
       * @brief Gets a unique ID of this Publisher
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
       * @return  The dump string. 
      **/
      ECAL_API_EXPORTED_MEMBER
        std::string Dump(const std::string& indent_ = "") const;

    private:
      // class members
      std::shared_ptr<CPublisherImpl> m_publisher_impl;
      long long                       m_filter_id;
    };
  }
}
