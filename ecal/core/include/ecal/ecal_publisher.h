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

#include <ecal/ecal_os.h>
#include <ecal/ecal_deprecate.h>
#include <ecal/ecal_callback.h>
#include <ecal/ecal_payload_writer.h>
#include <ecal/ecal_qos.h>
#include <ecal/ecal_tlayer.h>
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

    ECAL_API static constexpr long long DEFAULT_TIME_ARGUMENT        = -1;  /*!< Use DEFAULT_TIME_ARGUMENT in the `Send()` function to let eCAL determine the send timestamp */
    ECAL_API static constexpr long long DEFAULT_ACKNOWLEDGE_ARGUMENT = -1;  /*!< Use DEFAULT_ACKNOWLEDGE_ARGUMENT in the `Send()` function to let eCAL determine from configuration if the send operation needs to be acknowledged. */

    /**
     * @brief Constructor. 
    **/
    ECAL_API CPublisher();

    /**
     * @brief Constructor. 
     *
     * @param topic_name_   Unique topic name. 
     * @param topic_type_   Type name. 
     * @param topic_desc_   Type description (optional). 
    **/
    ECAL_DEPRECATE_SINCE_5_13("Please use the constructor CPublisher(const std::string& topic_name_, const SDataTypeInformation& topic_info_) instead. This function will be removed in eCAL6.")
    ECAL_API CPublisher(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_ = "");

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
    ECAL_API CPublisher(const std::string& topic_name_);

    /**
     * @brief Destructor. 
    **/
    ECAL_API virtual ~CPublisher();

    /**
     * @brief CPublishers are non-copyable
    **/
    ECAL_API CPublisher(const CPublisher&) = delete;

    /**
     * @brief CPublishers are non-copyable
    **/
    ECAL_API CPublisher& operator=(const CPublisher&) = delete;

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
     * @param topic_type_   Type name (optional). 
     * @param topic_desc_   Type description (optional). 
     *
     * @return  True if it succeeds, false if it fails. 
    **/
    ECAL_DEPRECATE_SINCE_5_13("Please use the create method bool Create(const std::string& topic_name_, const SDataTypeInformation& topic_info_) instead. This function will be removed in eCAL6.")
    ECAL_API bool Create(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_ = "");

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
    ECAL_API bool Create(const std::string& topic_name_)
    {
      return Create(topic_name_, SDataTypeInformation());
    }

    /**
     * @brief Destroys this object. 
     *
     * @return  True if it succeeds, false if it fails. 
    **/
    ECAL_API bool Destroy();

    /**
     * @brief Setup topic type name.
     *
     * @param topic_type_name_   Topic type name.
     *
     * @return  True if it succeeds, false if it fails.
    **/
    ECAL_DEPRECATE_SINCE_5_13("Please use the method bool SetDataTypeInformation(const SDataTypeInformation& topic_info_) instead. This function will be removed in eCAL6")
    ECAL_API bool SetTypeName(const std::string& topic_type_name_);

    /**
     * @brief Setup topic description. 
     *
     * @param topic_desc_   Description string. 
     *
     * @return  True if it succeeds, false if it fails. 
    **/
    ECAL_DEPRECATE_SINCE_5_13("Please use the method bool SetDataTypeInformation(const SDataTypeInformation& topic_info_) instead. This function will be removed in eCAL6")
    ECAL_API bool SetDescription(const std::string& topic_desc_);

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
     * @experimental
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
     * @brief Set publisher quality of service attributes.
     *
     * @param qos_  Quality of service policies.
     *
     * @return  True if it succeeds, false if it fails.
    **/
    ECAL_API bool SetQOS(const QOS::SWriterQOS& qos_);

    /**
     * @brief Get current publisher quality of service attributes.
     *
     * @return  Quality of service attributes.
    **/
    ECAL_API QOS::SWriterQOS GetQOS();

    /**
     * @brief Set publisher send mode for specific transport layer. 
     *
     * @param layer_  Transport layer.
     * @param mode_   Send mode.
     *
     * @return  True if it succeeds, false if it fails. 
    **/
    ECAL_API bool SetLayerMode(TLayer::eTransportLayer layer_, TLayer::eSendMode mode_);

    /**
     * @brief Set publisher maximum transmit bandwidth for the udp layer.
     *
     * @param bandwidth_  Maximum bandwidth in bytes/s (-1 == unlimited).
     *
     * @return  True if it succeeds, false if it fails.
    **/
    ECAL_API bool SetMaxBandwidthUDP(long bandwidth_);

    /**
     * @brief Set publisher maximum number of used shared memory buffers.
     *
     * @param buffering_  Maximum number of used buffers (needs to be greater than 1, default = 1).
     *
     * @return  True if it succeeds, false if it fails.
    **/
    ECAL_API bool ShmSetBufferCount(long buffering_);

    /**
     * @brief Enable zero copy shared memory transport mode.
     *
     * By default, the built-in shared memory layer is configured to make two memory copies
     * one on the publisher and one on the subscriber side.
     * 
     * The intention of this implementation is to free the file as fast as possible after writing and reading
     * its content to allow other processes to access the content with minimal latency. The publisher and subscribers
     * are fully decoupled and can access their internal memory copy independently.
     * 
     * If ShmEnableZeroCopy is switched on no memory will be copied at all using the low level binary publish / subscribe API.
     * On publisher side the memory copy is exectuted into the opened memory file. On the subscriber side the user message 
     * callback is called right after opening the memory file. A direct pointer to the memory payload is forwarded
     * and can be processed with no latency. The memory file will be closed after the user callback function
     * returned.
     *
     * The advantage of this configuration is a much higher performance for large payloads (> 1024 kB).
     * The disadvantage of this configuration is that in the time when the callback is executed the memory file 
     * is blocked for other subscribers and for writing publishers too. Maybe this can be eliminated
     * by a better memory file read/write access implementation (lock free read) in future releases.
     * 
     * Today, for specific scenarios (1:1 pub/sub connections with large payloads for example) this feature
     * can increase the performance remarkable. But please keep in mind to return from the message callback function
     * as fast as possible to not delay subsequent read/write access operations.
     * 
     * By using the eCAL::CPayloadWriter API a full zero copy implementation is possible by providing separate methods
     * for the initialization and the modification of the memory file content (see CPayloadWriter documentation).
     *
     * @param state_  Set type zero copy mode for shared memory transport layer (true == zero copy enabled).
     *
     * @return  True if it succeeds, false if it fails.
    **/
    ECAL_API bool ShmEnableZeroCopy(bool state_);

    /**
     * @brief Force connected subscribers to send acknowledge event after processing the message and 
     *        block publisher send call on this event with a timeout.
     *
     * Most applications perform very well with the default behavior. If subscribers are too slow 
     * to process incoming messages then the overall software architecture needs to be checked, software components 
     * need to be optimized or parallelized.
     * 
     * There may still be cases where it could make sense to synchronize the transfer of the payload from a publisher 
     * to a subscriber by using an additional handshake event. This event is signaled by a subscriber back to the 
     * sending publisher to confirm the complete payload transmission and the processed subscriber callback.
     * 
     * The publisher will wait up to the specified timeout for the acknowledge signals of all connected subscribers 
     * before sending new content. Finally that means the publishers CPublisher::Send API function call is now blocked 
     * and will not return until all subscriber have read and processed their content or the timeout has been reached.
     * 
     * @param acknowledge_timeout_ms_ timeout to wait for acknowledge signal from connected subscriber in ms (0 == no handshake).
     *
     * @return  True if it succeeds, false if it fails.
    **/
    ECAL_API bool ShmSetAcknowledgeTimeout(long long acknowledge_timeout_ms_);

    /**
     * @brief Force connected subscribers to send acknowledge event after processing the message and
     *        block publisher send call on this event with a timeout.
     *
     * See ShmSetAcknowledgeTimeout(long long acknowledge_timeout_ms_)
     * 
     * @param acknowledge_timeout_ timeout to wait for acknowledge signal from connected subscriber (0 == no handshake).
     *
     * @return  True if it succeeds, false if it fails.
    **/
    template <typename Rep, typename Period>
    bool ShmSetAcknowledgeTimeout(std::chrono::duration<Rep, Period> acknowledge_timeout_)
    {
      auto acknowledge_timeout_ms = std::chrono::duration_cast<std::chrono::milliseconds>(acknowledge_timeout_).count();
      return ShmSetAcknowledgeTimeout(static_cast<long long>(acknowledge_timeout_ms));
    }

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
     * @brief Send a message to all subscribers synchronized with acknowledge timeout (see also ShmSetAcknowledgeTimeout).
     * 
     * This synchronized mode is currently implemented for local interprocess communication (shm-ecal layer) only.
     *
     * @param buf_                     Pointer to content buffer.
     * @param len_                     Length of buffer.
     * @param time_                    Send time (-1 = use eCAL system time in us).
     * @param acknowledge_timeout_ms_  Maximum time to wait for all subscribers acknowledge feedback in ms (buffer received and processed).
     *
     * @return  Number of bytes sent.
    **/
    ECAL_API size_t Send(const void* buf_, size_t len_, long long time_, long long acknowledge_timeout_ms_) const;

    /**
     * @brief Send a message to all subscribers synchronized with acknowledge timeout (see also ShmSetAcknowledgeTimeout).
     * 
     * This synchronized mode is currently implemented for local interprocess communication (shm-ecal layer) only.
     *
     * @param buf_                     Pointer to content buffer.
     * @param len_                     Length of buffer.
     * @param time_                    Send time (-1 = use eCAL system time in us).
     * @param acknowledge_timeout_ms_  Maximum time to wait for all subscribers acknowledge feedback in ms (buffer received and processed).
     *
     * @return  Number of bytes sent.
    **/
    ECAL_DEPRECATE_SINCE_5_12("Please use the method size_t Send(CPayloadWriter& payload_, long long time_, long long acknowledge_timeout_ms_) const instead. This function will be removed in eCAL6.")
    ECAL_API size_t SendSynchronized(const void* const buf_, size_t len_, long long time_, long long acknowledge_timeout_ms_) const
    {
      return Send(buf_, len_, time_, acknowledge_timeout_ms_);
    }

    /**
     * @brief Send a message to all subscribers synchronized with acknowledge timeout (see also ShmSetAcknowledgeTimeout).
     *
     * This synchronized mode is currently implemented for local interprocess communication (shm-ecal layer) only.
     *
     * @param payload_                 Payload.
     * @param time_                    Send time (-1 = use eCAL system time in us).
     * @param acknowledge_timeout_ms_  Maximum time to wait for all subscribers acknowledge feedback in ms (buffer received and processed).
     *
     * @return  Number of bytes sent.
    **/
    ECAL_API size_t Send(CPayloadWriter& payload_, long long time_, long long acknowledge_timeout_ms_) const;

    /**
     * @brief Send a message to all subscribers.
     *
     * @param s_      String that contains content to send.
     * @param time_   Send time (-1 = use eCAL system time in us, default = -1).
     *
     * @return  Number of bytes sent.
    **/
    ECAL_API size_t Send(const std::string& s_, long long time_ = DEFAULT_TIME_ARGUMENT) const
    {
      return(Send(s_.data(), s_.size(), time_, DEFAULT_ACKNOWLEDGE_ARGUMENT));
    }

    /**
     * @brief Send a message to all subscribers synchronized.
     *
     * @param s_                       String that contains content to send.
     * @param time_                    Send time (-1 = use eCAL system time in us).
     * @param acknowledge_timeout_ms_  Maximum time to wait for all subscribers acknowledge feedback in ms (buffer received and processed).
     *
     * @return  Number of bytes sent.
    **/
    ECAL_API size_t Send(const std::string& s_, long long time_, long long acknowledge_timeout_ms_) const
    {
      return(Send(s_.data(), s_.size(), time_, acknowledge_timeout_ms_));
    }

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
     * @brief Dump the whole class state into a string. 
     *
     * @param indent_  Indentation used for dump. 
     *
     * @return  The dump string. 
    **/
    ECAL_API std::string Dump(const std::string& indent_ = "") const;

  protected:
    void InitializeQOS();
    void InitializeTLayer();
    bool ApplyTopicToDescGate(const std::string& topic_name_, const SDataTypeInformation& topic_info_);

    // class members
    std::shared_ptr<CDataWriter>     m_datawriter;
    struct ECAL_API QOS::SWriterQOS  m_qos;
    struct ECAL_API TLayer::STLayer  m_tlayer;
    long long                        m_id;
    bool                             m_created;
    bool                             m_initialized;
  };
};
