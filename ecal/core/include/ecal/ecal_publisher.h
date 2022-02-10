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
#include <ecal/ecal_callback.h>
#include <ecal/ecal_qos.h>
#include <ecal/ecal_tlayer.h>

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
  class ECAL_API CPublisher
  {
  public:
    /**
     * @brief Constructor. 
    **/
    CPublisher();

    /**
     * @brief Constructor. 
     *
     * @param topic_name_   Unique topic name. 
     * @param topic_type_   Type name (optional). 
     * @param topic_desc_   Type description (optional). 
    **/
    CPublisher(const std::string& topic_name_, const std::string& topic_type_ = "", const std::string& topic_desc_ = "");

    /**
     * @brief Destructor. 
    **/
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
    CPublisher(CPublisher&& rhs) noexcept;

    /**
     * @brief CPublishers are move-enabled
    **/
    CPublisher& operator=(CPublisher&& rhs) noexcept;

    /**
     * @brief Creates this object. 
     *
     * @param topic_name_   Unique topic name. 
     * @param topic_type_   Type name (optional). 
     * @param topic_desc_   Type description (optional). 
     *
     * @return  True if it succeeds, false if it fails. 
    **/
    bool Create(const std::string& topic_name_, const std::string& topic_type_ = "", const std::string& topic_desc_ = "");

    /**
     * @brief Destroys this object. 
     *
     * @return  True if it succeeds, false if it fails. 
    **/
    bool Destroy();

    /**
     * @brief Setup topic description. 
     *
     * @param topic_desc_   Description string. 
     *
     * @return  True if it succeeds, false if it fails. 
    **/
    bool SetDescription(const std::string& topic_desc_);

    /**
     * @brief Sets publisher attribute. 
     *
     * @param attr_name_   Attribute name. 
     * @param attr_value_  Attribute value. 
     *
     * @return  True if it succeeds, false if it fails. 
     * @experimental
    **/
    bool SetAttribute(const std::string& attr_name_, const std::string& attr_value_);

    /**
     * @brief Removes publisher attribute. 
     *
     * @param attr_name_   Attribute name. 
     *
     * @return  True if it succeeds, false if it fails.
     * @experimental
    **/
    bool ClearAttribute(const std::string& attr_name_);

    /**
     * @brief Share topic type.
     *
     * @param state_  Set type share mode (true == share type).
     *
     * @return  True if it succeeds, false if it fails.
    **/
    bool ShareType(bool state_ = true);

    /**
     * @brief Share topic description.
     *
     * @param state_  Set description share mode (true == share description).
     *
     * @return  True if it succeeds, false if it fails.
    **/
    bool ShareDescription(bool state_ = true);

    /**
     * @brief Set publisher quality of service attributes.
     *
     * @param qos_  Quality of service policies.
     *
     * @return  True if it succeeds, false if it fails.
    **/
    bool SetQOS(const QOS::SWriterQOS& qos_);

    /**
     * @brief Get current publisher quality of service attributes.
     *
     * @return  Quality of service attributes.
    **/
    QOS::SWriterQOS GetQOS();

    /**
     * @brief Set publisher send mode for specific transport layer. 
     *
     * @param layer_  Transport layer.
     * @param mode_   Send mode.
     *
     * @return  True if it succeeds, false if it fails. 
    **/
    bool SetLayerMode(TLayer::eTransportLayer layer_, TLayer::eSendMode mode_);

    /**
     * @brief Set publisher maximum transmit bandwidth for the udp layer.
     *
     * @param bandwidth_  Maximum bandwidth in bytes/s (-1 == unlimited).
     *
     * @return  True if it succeeds, false if it fails.
    **/
    bool SetMaxBandwidthUDP(long bandwidth_);

    /**
     * @brief Set publisher maximum number of used shared memory buffers.
     *
     * @param buffering_  Maximum number of used buffers (needs to be greater than 1, default = 1).
     *
     * @return  True if it succeeds, false if it fails.
    **/
    bool ShmSetBufferCount(long buffering_);

    /**
     * @brief Enable zero copy shared memory trasnport mode.
     *
     * By default, the builtin shared memory layer is configured to make one memory copy
     * on the receiver side. That means the payload is copied by the internal eCAL memory pool manager
     * out of the memory file and the file is closed immediately after this.
     * The intention of this implementation is to free the file as fast as possible after reading
     * its content to allow other subscribing processes to access the content with minimal latency.
     * The different reading subscribers are fully decoupled and can access their memory copy
     * independently.
     * 
     * If ShmEnableZeroCopy is switched on no memory will be copied at all. The user message callback is
     * called right after opening the memory file. A direct pointer to the memory payload is forwarded
     * and can be processed with no latency. The memory file will be closed after the user callback function
     * returned. The advantage of this configuration is a much higher performance for large payloads (> 1024 kB).
     * The disadvantage of this configuration is that in the time when the callback is executed the memory file 
     * is blocked for other subscribers and for writing publishers too. Maybe this can be eliminated
     * by a better memory file read/write access implementation (lock free read) in future releases.
     * 
     * Today, for specific scenarios (1:1 pub/sub connections with large payloads for example) this feature
     * can increase the performance remarkable. But please keep in mind to return from the message callback function
     * as fast as possible to not delay subsequent read/write access operations.
     *
     * @param state_  Set type zero copy mode for shared memory trasnport layer (true == zero copy enabled).
     *
     * @return  True if it succeeds, false if it fails.
    **/
    bool ShmEnableZeroCopy(bool state_);

    /**
     * @brief Set the the specific topic id.
     *
     * @param id_     The topic id for subscriber side filtering (0 == no id).
     *
     * @return  True if it succeeds, false if it fails.
    **/
    bool SetID(long long id_);

    /**
     * @brief Send a message to all subscribers. 
     *
     * @param buf_    Pointer to content buffer. 
     * @param len_    Length of buffer. 
     * @param time_   Send time (-1 = use eCAL system time in us, default = -1).
     *
     * @return  Number of bytes sent. 
    **/
    size_t Send(const void* const buf_, size_t len_, long long time_ = -1) const;

    /**
     * @brief Send a message to all subscribers. 
     *
     * @param s_      String that contains content to send. 
     * @param time_   Send time (-1 = use eCAL system time in us, default = -1).
     *
     * @return  Number of bytes sent. 
    **/
    size_t Send(const std::string& s_, long long time_ = -1) const;

    /**
     * @brief Add callback function for publisher events.
     *
     * @param type_      The event type to react on.
     * @param callback_  The callback function to add.
     *
     * @return  True if succeeded, false if not.
    **/
    bool AddEventCallback(eCAL_Publisher_Event type_, PubEventCallbackT callback_);

    /**
     * @brief Remove callback function for publisher events.
     *
     * @param type_  The event type to remove.
     *
     * @return  True if succeeded, false if not.
    **/
    bool RemEventCallback(eCAL_Publisher_Event type_);

    /**
     * @brief Query if the publisher is created. 
     *
     * @return  True if created, false if not. 
    **/
    bool IsCreated() const {return(m_created);}

    /**
     * @brief Query if the publisher is subscribed. 
     *
     * @return  true if subscribed, false if not. 
    **/
    bool IsSubscribed() const;

    /**
     * @brief Query the number of subscribers. 
     *
     * @return  Number of subscribers. 
    **/
    size_t GetSubscriberCount() const;

    /**
     * @brief Gets name of the connected topic. 
     *
     * @return  The topic name. 
    **/
    std::string GetTopicName() const;

    /**
     * @brief Gets type of the connected topic. 
     *
     * @return  The type name. 
    **/
    std::string GetTypeName() const;

    /**
     * @brief Gets description of the connected topic. 
     *
     * @return  The description. 
    **/
    std::string GetDescription() const;

    /**
     * @brief Dump the whole class state into a string. 
     *
     * @param indent_  Indentation used for dump. 
     *
     * @return  The dump string. 
    **/
    std::string Dump(const std::string& indent_ = "") const;

  protected:
    void InitializeQOS();
    void InitializeTLayer();

    // class members
    CDataWriter*                     m_datawriter;
    struct ECAL_API QOS::SWriterQOS  m_qos;
    struct ECAL_API TLayer::STLayer  m_tlayer;
    long long                        m_id;
    bool                             m_created;
    bool                             m_initialized;
  };
};
