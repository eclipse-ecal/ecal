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
 * @brief  eCAL publisher interface
**/

#pragma once

#include <cstddef>
#include <ecal/ecal_callback.h>
#include <ecal/ecal_deprecate.h>
#include <ecal/ecal_os.h>
#include <ecal/ecal_payload_writer.h>
#include <ecal/ecal_types.h>

#include <chrono>
#include <memory>
#include <string>

namespace eCAL
{
  class CDataWriter;

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
  class CPublisher
  {
  public:

    ECAL_API static constexpr long long DEFAULT_TIME_ARGUMENT = -1;  /*!< Use DEFAULT_TIME_ARGUMENT in the `Send()` function to let eCAL determine the send timestamp */

    /**
     * @brief Constructor. 
    **/
    ECAL_API CPublisher();

    /**
     * @brief Constructor.
     *
     * @param topic_name_   Unique topic name.
     * @param topic_info_   Topic information (encoding, type, descriptor)
    **/
    ECAL_API CPublisher(const std::string& topic_name_, const SDataTypeInformation& topic_info_);

    /**
     * @brief Constructor.
     *
     * @param topic_name_   Unique topic name.
    **/
    ECAL_API explicit CPublisher(const std::string& topic_name_);

    /**
     * @brief Destructor. 
    **/
    ECAL_API virtual ~CPublisher();

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
    ECAL_API CPublisher(CPublisher&& rhs) noexcept;

    /**
     * @brief CPublishers are move-enabled
    **/
    ECAL_API CPublisher& operator=(CPublisher&& rhs) noexcept;

    /**
     * @brief Creates this object.
     *
     * @param topic_name_   Unique topic name.
     * @param topic_info_   Topic information (encoding, type, descriptor)
     *
     * @return  True if it succeeds, false if it fails.
    **/
    ECAL_API bool Create(const std::string& topic_name_, const SDataTypeInformation& topic_info_);

    /**
     * @brief Creates this object.
     *
     * @param topic_name_   Unique topic name.
     *
     * @return  True if it succeeds, false if it fails.
    **/
    ECAL_API bool Create(const std::string& topic_name_);

    /**
     * @brief Destroys this object. 
     *
     * @return  True if it succeeds, false if it fails. 
    **/
    ECAL_API bool Destroy();

    /**
     * @brief Setup topic information.
     *
     * @param topic_info_  Topic information attributes.
     *
     * @return  True if it succeeds, false if it fails.
    **/
    ECAL_API bool SetDataTypeInformation(const SDataTypeInformation& topic_info_);

    /**
     * @brief Sets publisher attribute. 
     *
     * @param attr_name_   Attribute name. 
     * @param attr_value_  Attribute value. 
     *
     * @return  True if it succeeds, false if it fails. 
    **/
    ECAL_API bool SetAttribute(const std::string& attr_name_, const std::string& attr_value_);

    /**
     * @brief Removes publisher attribute. 
     *
     * @param attr_name_   Attribute name. 
     *
     * @return  True if it succeeds, false if it fails.
     * @experimental
    **/
    ECAL_API bool ClearAttribute(const std::string& attr_name_);

    /**
     * @brief Share topic type.
     *
     * @param state_  Set type share mode (true == share type).
     *
     * @return  True if it succeeds, false if it fails.
    **/
    ECAL_API bool ShareType(bool state_ = true);

    /**
     * @brief Share topic description.
     *
     * @param state_  Set description share mode (true == share description).
     *
     * @return  True if it succeeds, false if it fails.
    **/
    ECAL_API bool ShareDescription(bool state_ = true);

    /**
     * @brief Set the specific topic id.
     *
     * @param id_     The topic id for subscriber side filtering (0 == no id).
     *
     * @return  True if it succeeds, false if it fails.
    **/
    ECAL_API bool SetID(long long id_);

    /**
     * @brief Send a message to all subscribers. 
     *
     * @param buf_    Pointer to content buffer. 
     * @param len_    Length of buffer. 
     * @param time_   Send time (-1 = use eCAL system time in us, default = -1).
     *
     * @return  Number of bytes sent. 
    **/
    ECAL_API size_t Send(const void* buf_, size_t len_, long long time_ = DEFAULT_TIME_ARGUMENT) const;

    /**
     * @brief Send a message to all subscribers.
     *
     * @param payload_  Payload.
     * @param time_     Send time (-1 = use eCAL system time in us, default = -1).
     *
     * @return  Number of bytes sent.
    **/
    ECAL_API size_t Send(CPayloadWriter& payload_, long long time_ = DEFAULT_TIME_ARGUMENT) const;

    /**
     * @brief Send a message to all subscribers.
     *
     * @param s_      String that contains content to send.
     * @param time_   Send time (-1 = use eCAL system time in us, default = -1).
     *
     * @return  Number of bytes sent.
    **/
    ECAL_API size_t Send(const std::string& s_, long long time_ = DEFAULT_TIME_ARGUMENT) const;

    /**
     * @brief Add callback function for publisher events.
     *
     * @param type_      The event type to react on.
     * @param callback_  The callback function to add.
     *
     * @return  True if succeeded, false if not.
    **/
    ECAL_API bool AddEventCallback(eCAL_Publisher_Event type_, PubEventCallbackT callback_);

    /**
     * @brief Remove callback function for publisher events.
     *
     * @param type_  The event type to remove.
     *
     * @return  True if succeeded, false if not.
    **/
    ECAL_API bool RemEventCallback(eCAL_Publisher_Event type_);

    /**
     * @brief Query if the publisher is created. 
     *
     * @return  True if created, false if not. 
    **/
    ECAL_API bool IsCreated() const {return(m_created);}

    /**
     * @brief Query if the publisher is subscribed. 
     *
     * @return  true if subscribed, false if not. 
    **/
    ECAL_API bool IsSubscribed() const;

    /**
     * @brief Query the number of subscribers.
     *
     * @return  Number of subscribers.
    **/
    ECAL_API size_t GetSubscriberCount() const;

    /**
     * @brief Gets name of the connected topic. 
     *
     * @return  The topic name. 
    **/
    ECAL_API std::string GetTopicName() const;

    /**
     * @brief Gets description of the connected topic.
     *
     * @return  The topic information.
    **/
    ECAL_API SDataTypeInformation GetDataTypeInformation() const;

    /**
     * @brief Dump the whole class state into a string. 
     *
     * @param indent_  Indentation used for dump. 
     *
     * @return  The dump string. 
    **/
    ECAL_API std::string Dump(const std::string& indent_ = "") const;

  protected:
    // class members
    std::shared_ptr<CDataWriter>     m_datawriter;
    long long                        m_id;
    bool                             m_created;
    bool                             m_initialized;
  };
}
