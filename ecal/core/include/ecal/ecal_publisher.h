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

#ifndef ECAL_C_DLL

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
     * @state_  Set type zero copy mode for shared memory trasnport layer (true == zero copy enabled).
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

#else /* ! ECAL_C_DLL */

#include <ecal/ecalc.h>

namespace eCAL
{
  class CPublisher
  {
  public:
    CPublisher() :
      m_publisher(nullptr),
      m_id(0),
      m_created(false)
    {
      InitializeQOS();
      InitializeTLayer();
    }

    CPublisher(const std::string& topic_name_, const std::string& topic_type_ = "", const std::string& topic_desc_ = "") :
      m_publisher(nullptr),
      m_id(0),
      m_created(false)
    {
      InitializeQOS();
      InitializeTLayer();

      Create(topic_name_, topic_type_, topic_desc_);
    }

    virtual ~CPublisher()
    {
      Destroy();
    }

    // this object must not be copied.
    CPublisher(const CPublisher&) = delete;
    CPublisher& operator=(const CPublisher&) = delete;

    /**
    * @brief CSubscribers are move-enabled
    **/
    CPublisher(CPublisher&& rhs)
      : m_publisher(rhs.m_publisher)
      , m_topic_name(std::move(rhs.m_topic_name))
      , m_topic_type(std::move(rhs.m_topic_type))
      , m_topic_desc(std::move(rhs.m_topic_desc))
      , m_qos(rhs.m_qos)
      , m_tlayer(rhs.m_tlayer)
      , m_event_callback(std::move(rhs.m_event_callback))
      , m_id(rhs.m_id)
      , m_created(rhs.m_created)
    {
      InitializeQOS();
      InitializeTLayer();

      rhs.m_publisher = nullptr;
      rhs.m_created   = false;
    }

    /**
    * @brief CSubscribers are move-enabled
    **/
    CPublisher& operator=(CPublisher&& rhs)
    {
      m_publisher      = rhs.m_publisher;
      m_topic_name     = std::move(rhs.m_topic_name);
      m_topic_type     = std::move(rhs.m_topic_type);
      m_topic_desc     = std::move(rhs.m_topic_desc);
      m_qos            = rhs.m_qos;
      m_tlayer         = rhs.m_tlayer;
      m_event_callback = std::move(rhs.m_event_callback);
      m_id             = rhs.m_id;
      m_created        = rhs.m_created;

      InitializeQOS();
      InitializeTLayer();
      rhs.m_publisher = nullptr;
      rhs.m_created   = false;

      return *this;
    }

    bool Create(const std::string& topic_name_, const std::string& topic_type_ = "", const std::string& topic_desc_ = "")
    {
      if(m_created) return(false);

      // create new instance
      m_publisher = eCAL_Pub_New();
      if (!m_publisher) return(false);

      // set layer mode if changed class method function
      if (m_tlayer.sm_udp_mc != TLayer::smode_none) eCAL_Pub_SetLayerMode(m_publisher, static_cast<eTransportLayerC>(TLayer::tlayer_udp_mc), static_cast<eSendModeC>(m_tlayer.sm_udp_mc));
      if (m_tlayer.sm_shm    != TLayer::smode_none) eCAL_Pub_SetLayerMode(m_publisher, static_cast<eTransportLayerC>(TLayer::tlayer_shm),    static_cast<eSendModeC>(m_tlayer.sm_shm));

      // set qos
      struct SWriterQOSC qos;
      qos.history_kind       = static_cast<eQOSPolicy_HistoryKindC>(m_qos.history_kind);
      qos.history_kind_depth = m_qos.history_kind_depth;;
      qos.reliability        = static_cast<eQOSPolicy_ReliabilityC>(m_qos.reliability);
      eCAL_Pub_SetQOS(m_publisher, qos);

      // create it
      eCAL_Pub_Create(m_publisher, topic_name_.c_str(), topic_type_.c_str(), topic_desc_.c_str(), static_cast<int>(topic_desc_.size()));

      m_topic_name = topic_name_;
      m_topic_type = topic_type_;
      m_topic_desc = topic_desc_;

      m_created = true;
      return(true);
    }

    bool Destroy()
    {
      if(!m_created) return(false);
      if(m_publisher) eCAL_Pub_Destroy(m_publisher);

      m_publisher  = nullptr;
      m_topic_name = "";
      m_topic_type = "";
      m_created    = false;
      return(true);
    }

    bool SetDescription(const std::string& topic_desc_)
    {
      m_topic_desc = topic_desc_;
      if(!m_publisher) return(false);
      return(eCAL_Pub_SetDescription(m_publisher, topic_desc_.c_str(), static_cast<int>(topic_desc_.size())) != 0);
    }

    bool ShareType(bool state_ = true)
    {
      if (!m_publisher) return(false);
      return(eCAL_Pub_ShareType(m_publisher, static_cast<int>(state_)) != 0);
    }

    bool ShareDescription(bool state_ = true)
    {
      if (!m_publisher) return(false);
      return(eCAL_Pub_ShareDescription(m_publisher, static_cast<int>(state_)) != 0);
    }

    bool SetQOS(const QOS::SWriterQOS& qos_)
    {
      // can not be set after Create
      if (m_publisher) return(false);
      m_qos = qos_;
      return(true);
    }

    QOS::SWriterQOS GetQOS()
    {
      return(m_qos);
    }

    bool SetLayerMode(TLayer::eTransportLayer layer_, TLayer::eSendMode mode_)
    {
      if (!m_publisher)
      {
        switch (layer_)
        {
        case TLayer::tlayer_udp_mc:
          m_tlayer.sm_udp_mc = mode_;
          break;
        case TLayer::tlayer_shm:
          m_tlayer.sm_shm = mode_;
          break;
        case TLayer::tlayer_inproc:
          m_tlayer.sm_inproc = mode_;
          break;
        case TLayer::tlayer_all:
          m_tlayer.sm_udp_mc  = mode_;
          m_tlayer.sm_shm     = mode_;
          m_tlayer.sm_inproc  = mode_;
          break;
        default:
          break;
        }
        return(true);
      }
      else
      {
        return(eCAL_Pub_SetLayerMode(m_publisher, static_cast<eTransportLayerC>(layer_), static_cast<eSendModeC>(mode_)) != 0);
      }
    }

    bool SetMaxBandwidthUDP(long bandwidth_)
    {
      if (!m_publisher) return(false);
      return(eCAL_Pub_SetMaxBandwidthUDP(m_publisher, bandwidth_) != 0);
    }

    bool SetID(long long id_)
    {
      if (!m_publisher) return(false);
      return(eCAL_Pub_SetID(m_publisher, id_) != 0);
    }

    size_t Send(const void* const buf_, size_t len_, long long time_ = -1) const
    {
      if(!m_publisher) return(0);
      return(eCAL_Pub_Send(m_publisher, buf_, static_cast<int>(len_), time_));
    }

    size_t Send(const std::string& s_, long long time_ = -1) const
    {
      return(eCAL_Pub_Send(m_publisher, s_.data(), static_cast<int>(s_.size()), time_));
    }

    static void PubEventCallback(const char* topic_name_, const struct SPubEventCallbackDataC* data_, void* par_)
    {
      if (par_ == nullptr) return;
      CPublisher* pub = static_cast<CPublisher*>(par_);
      SPubEventCallbackData data;
      data.type = data_->type;
      data.time = data_->time;
      pub->m_event_callback(topic_name_, &data);
    }
    bool AddEventCallback(eCAL_Publisher_Event type_, PubEventCallbackT callback_)
    {
      if (!m_publisher) return(false);
      m_event_callback = callback_;
      return(eCAL_Pub_AddEventCallbackC(m_publisher, type_, &CPublisher::PubEventCallback, this) != 0);
    }

    bool RemEventCallback(eCAL_Publisher_Event type_)
    {
      if (!m_publisher) return(false);
      return(eCAL_Pub_RemEventCallback(m_publisher, type_) != 0);
    }

    bool IsCreated() const {return(m_created);}

    bool IsSubscribed() const
    {
      if(!m_publisher) return(false);
      return(eCAL_Pub_IsSubscribed(m_publisher) != 0);
    }

    const std::string& GetTopicName() const {return(m_topic_name);}

    const std::string& GetTypeName() const {return(m_topic_type);}

    const std::string& GetDescription() const {return(m_topic_desc);}

    void InitializeQOS()
    {
      m_qos.history_kind       = QOS::keep_last_history_qos;
      m_qos.history_kind_depth = 8;
      m_qos.reliability        = QOS::reliable_reliability_qos;
    }

    void InitializeTLayer()
    {
      m_tlayer.sm_udp_mc  = TLayer::smode_none;
      m_tlayer.sm_shm     = TLayer::smode_none;
      m_tlayer.sm_inproc  = TLayer::smode_none;
    }

    std::string Dump() const
    {
      if(!m_publisher) return("");
      void* buf = nullptr;
      size_t buf_len = eCAL_Pub_Dump(m_publisher, &buf, ECAL_ALLOCATE_4ME);
      if(buf_len == 0) return("");
      std::string dbuf = std::string(static_cast<char*>(buf), buf_len);
      eCAL_FreeMem(buf);
      return(dbuf);
    }

  protected:
    // class members
    ECAL_HANDLE             m_publisher;
    std::string             m_topic_name;
    std::string             m_topic_type;
    std::string             m_topic_desc;
    QOS::SWriterQOS         m_qos;
    TLayer::STLayer         m_tlayer;
    PubEventCallbackT       m_event_callback;
    long long               m_id;
    bool                    m_created;
  };
};

#endif /* ! ECAL_C_DLL */
