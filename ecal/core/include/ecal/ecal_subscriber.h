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
 * @file   ecal_subscriber.h
 * @brief  eCAL subscriber interface
**/

#pragma once

#include <ecal/ecal_os.h>
#include <ecal/ecal_deprecate.h>
#include <ecal/ecal_callback.h>
#include <ecal/ecal_qos.h>
#include <ecal/ecal_types.h>

#include <memory>
#include <set>
#include <string>

namespace eCAL
{
  class CDataReader;

  /**
   * @brief eCAL subscriber class.
   *
   * The CSubscriber class is used to receive topics from matching eCAL publishers.
   * <br>
   * <br>
   * For receiving the topic payload the subscriber class provides a callback interface and overloaded receive methods.
  **/

  /**
   * @code
   *            // create subscriber, topic name "A"
   *            eCAL::CSubscriber sub("A");
   *
   *            //---------------------------------------------------------------------------------------
   *            // receive content with 100 ms timeout as standard string and read receive time stamp
   *            //---------------------------------------------------------------------------------------
   *            std::string rcv_s;
   *            long long time = 0;
   *            size_t rcv_len = sub.Receive(rcv_s, &time, 100);
   *
   *            //---------------------------------------------------------------------------------------
   *            // receive content with a callback functions
   *            //---------------------------------------------------------------------------------------
   *            void OnReceive(const char* topic_name_, const struct eCAL::SReceiveCallbackData* data_)
   *            {
   *              size_t      size = data_->size;
   *              const void* data = data_->buf;
   *              long long   time = data_->time;
   *            }
   *
   *            // add callback
   *            sub.AddReceiveCallback(std::bind(OnReceive, std::placeholders::_1, std::placeholders::_2));
   *
   *            // idle main thread
   *            while(eCAL::Ok())
   *            {
   *              // sleep 100 ms
   *              std::this_thread::sleep_for(std::chrono::milliseconds(100));
   *            }
   * @endcode
  **/

  class CSubscriber
  {
  public:
    /**
     * @brief Constructor. 
    **/
    ECAL_API CSubscriber();

    /**
     * @brief Constructor. 
     *
     * @param topic_name_   Unique topic name. 
     * @param topic_type_   Type name (optional for type checking).
     * @param topic_desc_   Type description (optional for description checking).
     **/
    ECAL_DEPRECATE_SINCE_5_13("Please use the constructor CSubscriber(const std::string& topic_name_, const SDataTypeInformation& topic_info_) instead. This function will be removed in eCAL6.")
    ECAL_API CSubscriber(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_ = "");

    /**
    * @brief Constructor.
    *
    * @param topic_name_   Unique topic name.
    * @param topic_info_   Topic information (encoding, type, descriptor)
    **/
    ECAL_API CSubscriber(const std::string& topic_name_, const SDataTypeInformation& topic_info_);

    /**
    * @brief Constructor.
    * 
    * @param topic_name_   Unique topic name.
    **/
    ECAL_API CSubscriber(const std::string& topic_name_);

    /**
     * @brief Destructor. 
    **/
    ECAL_API virtual ~CSubscriber();

    /**
     * @brief CSubscribers are non-copyable
    **/
    ECAL_API CSubscriber(const CSubscriber&) = delete;

    /**
     * @brief CSubscribers are non-copyable
    **/
    ECAL_API CSubscriber& operator=(const CSubscriber&) = delete;

    /**
     * @brief CSubscribers are move-enabled
    **/
    ECAL_API CSubscriber(CSubscriber&& rhs) noexcept;

    /**
     * @brief CSubscribers are move-enabled
    **/
    ECAL_API CSubscriber& operator=(CSubscriber&& rhs) noexcept;

    /**
     * @brief Creates this object. 
     *
     * @param topic_name_   Unique topic name. 
     * @param topic_type_   Type name (optional for type checking).
     * @param topic_desc_   Type description (optional for description checking).
     *
     * @return  true if it succeeds, false if it fails. 
    **/
    ECAL_DEPRECATE_SINCE_5_13("Please use the create method bool Create(const std::string& topic_name_, const STopicInformation& topic_info_) instead. This function will be removed in eCAL6.")
    ECAL_API bool Create(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_ = "");

    /**
     * @brief Creates this object.
     *
     * @param topic_name_   Unique topic name.
     *
     * @return  True if it succeeds, false if it fails.
    **/
    ECAL_API bool Create(const std::string& topic_name_) {
      return Create(topic_name_, SDataTypeInformation{});
    }

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
     * @brief Destroys this object. 
     *
     * @return  true if it succeeds, false if it fails. 
    **/
    ECAL_API bool Destroy();

    /**
     * @brief Set subscriber quality of service attributes.
     *
     * @param qos_  Quality of service policies.
     *
     * @return  True if it succeeds, false if it fails.
    **/
    ECAL_API bool SetQOS(const QOS::SReaderQOS& qos_);

    /**
     * @brief Get current subscriber quality of service attributes.
     *
     * @return  Quality of service attributes.
    **/
    ECAL_API QOS::SReaderQOS GetQOS();

    /**
     * @brief Set a set of id's to prefiltering topics (see CPublisher::SetID).
     *
     * @param id_set_  Set of id's.
     *
     * @return  True if it succeeds, false if it fails.
    **/
    ECAL_API bool SetID(const std::set<long long>& id_set_);

    /**
     * @brief Sets subscriber attribute. 
     *
     * @param attr_name_   Attribute name. 
     * @param attr_value_  Attribute value. 
     *
     * @return  True if it succeeds, false if it fails. 
     * @experimental
    **/
    ECAL_API bool SetAttribute(const std::string& attr_name_, const std::string& attr_value_);

    /**
     * @brief Removes subscriber attribute. 
     *
     * @param attr_name_   Attribute name. 
     *
     * @return  True if it succeeds, false if it fails.
     * @experimental
    **/
    ECAL_API bool ClearAttribute(const std::string& attr_name_);

    /**
     * @brief Receive a message from the publisher. 
     *
     * @param [out] buf_    Standard string for copying message content.
     * @param [out] time_   Time from publisher in us (default = nullptr).
     * @param rcv_timeout_  Maximum time before receive operation returns (in milliseconds, -1 means infinite).
     *
     * @return  Length of received buffer. 
    **/
    ECAL_DEPRECATE_SINCE_5_10("Please use the method bool ReceiveBuffer(std::string& buf_, long long* time_ = nullptr, int rcv_timeout_ = 0) instead. This function will be removed in eCAL6.")
    ECAL_API size_t Receive(std::string& buf_, long long* time_ = nullptr, int rcv_timeout_ = 0) const;

    /**
     * @brief Receive a message from the publisher (able to process zero length buffer).
     *
     * @param [out] buf_    Standard string for copying message content.
     * @param [out] time_   Time from publisher in us (default = nullptr).
     * @param rcv_timeout_  Maximum time before receive operation returns (in milliseconds, -1 means infinite).
     *
     * @return  True if it succeeds, false if it fails.
    **/
    ECAL_API bool ReceiveBuffer(std::string& buf_, long long* time_ = nullptr, int rcv_timeout_ = 0) const;

    /**
     * @brief Add callback function for incoming receives. 
     *
     * @param callback_  The callback function to add.
     *
     * @return  True if succeeded, false if not. 
    **/
    ECAL_API bool AddReceiveCallback(ReceiveCallbackT callback_);

    /**
     * @brief Remove callback function for incoming receives. 
     *
     * @return  True if succeeded, false if not. 
    **/
    ECAL_API bool RemReceiveCallback();

    /**
     * @brief Add callback function for subscriber events.
     *
     * @param type_      The event type to react on.
     * @param callback_  The callback function to add.
     *
     * @return  True if succeeded, false if not.
    **/
    ECAL_API bool AddEventCallback(eCAL_Subscriber_Event type_, SubEventCallbackT callback_);

    /**
     * @brief Remove callback function for subscriber events.
     *
     * @param type_  The event type to remove.
     *
     * @return  True if succeeded, false if not.
    **/
    ECAL_API bool RemEventCallback(eCAL_Subscriber_Event type_);

    /**
     * @brief Query if this object is created. 
     *
     * @return  true if created, false if not. 
    **/
    ECAL_API bool IsCreated() const {return(m_created);}

    /**
     * @brief Query the number of publishers. 
     *
     * @return  Number of publishers. 
    **/
    ECAL_API size_t GetPublisherCount() const;

    /**
     * @brief Gets name of the connected topic. 
     *
     * @return  The topic name. 
    **/
    ECAL_API std::string GetTopicName() const;

    /**
     * @brief Gets type of the connected topic. 
     *
     * @return  The type name. 
    **/
    ECAL_DEPRECATE_SINCE_5_13("Please use the method SDataTypeInformation GetDataTypeInformation() instead. You can extract the typename from the STopicInformation variable. This function will be removed in eCAL6.")
    ECAL_API std::string GetTypeName() const;

    /**
     * @brief Gets description of the connected topic. 
     *
     * @return  The description. 
    **/
    ECAL_DEPRECATE_SINCE_5_13("Please use the method SDataTypeInformation GetDataTypeInformation() instead. You can extract the descriptor from the STopicInformation variable. This function will be removed in eCAL6.")
    ECAL_API std::string GetDescription() const;

    /**
    * @brief Gets description of the connected topic.
    *
    * @return  The topic information.
    **/
    ECAL_API SDataTypeInformation GetDataTypeInformation() const;

    /**
     * @brief Set the timeout parameter for triggering
     *          the timeout callback.
     *
     * @param timeout_  The timeout in milliseconds.
     *
     * @return  True if succeeded, false if not.
    **/
    ECAL_API bool SetTimeout(int timeout_);

    /**
     * @brief Dump the whole class state into a string. 
     *
     * @param indent_  Indentation used for dump. 
     *
     * @return  The dump sting. 
    **/
    ECAL_API std::string Dump(const std::string& indent_ = "") const;

  protected:
    void InitializeQOS();
    bool ApplyTopicToDescGate(const std::string& topic_name_, const SDataTypeInformation& topic_info_);

    // class members
    std::shared_ptr<CDataReader>     m_datareader;
    struct ECAL_API QOS::SReaderQOS  m_qos;
    bool                             m_created;
    bool                             m_initialized;
  };
};
