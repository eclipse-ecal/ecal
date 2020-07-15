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
#include <ecal/ecal_callback.h>
#include <ecal/ecal_qos.h>

#include <set>
#include <string>

#ifndef ECAL_C_DLL

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

  class ECAL_API CSubscriber
  {
  public:
    /**
     * @brief Constructor. 
    **/
    CSubscriber();

    /**
     * @brief Constructor. 
     *
     * @param topic_name_   Unique topic name. 
     * @param topic_type_   Type name (optional for type checking).
     * @param topic_desc_   Type description (optional for description checking).
     **/
    CSubscriber(const std::string& topic_name_, const std::string& topic_type_ = "", const std::string& topic_desc_ = "");

    /**
     * @brief Destructor. 
    **/
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
    CSubscriber(CSubscriber&& rhs);

    /**
     * @brief CSubscribers are move-enabled
    **/
    CSubscriber& operator=(CSubscriber&& rhs);

    /**
     * @brief Creates this object. 
     *
     * @param topic_name_   Unique topic name. 
     * @param topic_type_   Type name (optional for type checking).
     * @param topic_desc_   Type description (optional for description checking).
     *
     * @return  true if it succeeds, false if it fails. 
    **/
    bool Create(const std::string& topic_name_, const std::string& topic_type_ = "", const std::string& topic_desc_ = "");

    /**
     * @brief Destroys this object. 
     *
     * @return  true if it succeeds, false if it fails. 
    **/
    bool Destroy();

    /**
     * @brief Set subscriber quality of service attributes.
     *
     * @param qos_  Quality of service policies.
     *
     * @return  True if it succeeds, false if it fails.
    **/
    bool SetQOS(const QOS::SReaderQOS& qos_);

    /**
     * @brief Get current subscriber quality of service attributes.
     *
     * @return  Quality of service attributes.
    **/
    QOS::SReaderQOS GetQOS();

    /**
     * @brief Set a set of id's to prefiltering topics (see CPublisher::SetID).
     *
     * @param id_set_  Set of id's.
     *
     * @return  True if it succeeds, false if it fails.
    **/
    bool SetID(const std::set<long long>& id_set_);

    /**
     * @brief Receive a message from the publisher. 
     *
     * @param [out] buf_    Standard string for copying message content.
     * @param [out] time_   Time from publisher in us (default = nullptr).
     * @param rcv_timeout_  Maximum time before receive operation returns (in milliseconds, -1 means infinite).
     *
     * @return  Length of received buffer. 
    **/
    size_t Receive(std::string& buf_, long long* time_ = nullptr, int rcv_timeout_ = 0) const;

    /**
     * @brief Add callback function for incoming receives. 
     *
     * @param callback_  The callback function to add.
     *
     * @return  True if succeeded, false if not. 
    **/
    bool AddReceiveCallback(ReceiveCallbackT callback_);

    /**
     * @brief Remove callback function for incoming receives. 
     *
     * @return  True if succeeded, false if not. 
    **/
    bool RemReceiveCallback();

    /**
     * @brief Add callback function for subscriber events.
     *
     * @param type_      The event type to react on.
     * @param callback_  The callback function to add.
     *
     * @return  True if succeeded, false if not.
    **/
    bool AddEventCallback(eCAL_Subscriber_Event type_, SubEventCallbackT callback_);

    /**
     * @brief Remove callback function for subscriber events.
     *
     * @param type_  The event type to remove.
     *
     * @return  True if succeeded, false if not.
    **/
    bool RemEventCallback(eCAL_Subscriber_Event type_);

    /**
     * @brief Query if this object is created. 
     *
     * @return  true if created, false if not. 
    **/
    bool IsCreated() const {return(m_created);}

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
     * @brief Set the timeout parameter for triggering
     *          the timeout callback.
     *
     * @param timeout_  The timeout in milliseconds.
     *
     * @return  True if succeeded, false if not.
    **/
    bool SetTimeout(int timeout_);

    /**
     * @brief Dump the whole class state into a string. 
     *
     * @param indent_  Indentation used for dump. 
     *
     * @return  The dump sting. 
    **/
    std::string Dump(const std::string& indent_ = "") const;

  protected:
    void InitializeQOS();

    // class members
    CDataReader*                     m_datareader;
    struct ECAL_API QOS::SReaderQOS  m_qos;
    bool                             m_created;
    bool                             m_initialized;
  };
};

#else /* ! ECAL_C_DLL */

#include <ecal/ecalc.h>

namespace eCAL
{
  class CSubscriber
  {
  public:
    CSubscriber() :
      m_subscriber(nullptr),
      m_created(false)
    {
      InitializeQOS();
    }

    CSubscriber(const std::string& topic_name_, const std::string& topic_type_ = "", const std::string& topic_desc_ = "") :
      m_subscriber(nullptr),
      m_created(false)
    {
      InitializeQOS();

      Create(topic_name_, topic_type_, topic_desc_);
    }

    virtual ~CSubscriber()
    {
      Destroy();
    }

    // this object must not be copied.
    CSubscriber(const CSubscriber&) = delete;
    CSubscriber& operator=(const CSubscriber&) = delete;

    CSubscriber(CSubscriber&& rhs)
      : m_subscriber(rhs.m_subscriber)
      , m_topic_name(std::move(rhs.m_topic_name))
      , m_topic_type(std::move(rhs.m_topic_type))
      , m_topic_desc(std::move(rhs.m_topic_desc))
      , m_qos(rhs.m_qos)
      , m_receive_callback(std::move(rhs.m_receive_callback))
      , m_event_callback(std::move(rhs.m_event_callback))
      , m_created(rhs.m_created)
    {
      InitializeQOS();

      rhs.m_subscriber = nullptr;
      rhs.m_created    = false;
    }

    CSubscriber& operator=(CSubscriber&& rhs)
    {
      m_subscriber       = rhs.m_subscriber;
      m_topic_name       = std::move(rhs.m_topic_name);
      m_topic_type       = std::move(rhs.m_topic_type);
      m_topic_desc       = std::move(rhs.m_topic_desc);
      m_qos              = rhs.m_qos;
      m_receive_callback = std::move(rhs.m_receive_callback);
      m_event_callback   = std::move(rhs.m_event_callback);
      m_created          = rhs.m_created;

      InitializeQOS();
      rhs.m_subscriber = nullptr;
      rhs.m_created    = false;

      return *this;
    }

    bool Create(const std::string& topic_name_, const std::string& topic_type_ = "", const std::string& topic_desc_ = "")
    {
      if(m_created) return(false);

      // create new instance
      m_subscriber = eCAL_Sub_New();
      if (!m_subscriber) return(false);

      // set qos
      struct SReaderQOSC qos;
      qos.history_kind       = static_cast<eQOSPolicy_HistoryKindC>(m_qos.history_kind);
      qos.history_kind_depth = m_qos.history_kind_depth;;
      qos.reliability        = static_cast<eQOSPolicy_ReliabilityC>(m_qos.reliability);
      eCAL_Sub_SetQOS(m_subscriber, qos);

      // create it
      eCAL_Sub_Create(m_subscriber, topic_name_.c_str(), topic_type_.c_str(), topic_desc_.c_str(), static_cast<int>(topic_desc_.size()));

      m_topic_name = topic_name_;
      m_topic_type = topic_type_;
      m_topic_desc = topic_desc_;

      m_created = true;
      return(true);
    }

    bool Destroy()
    {
      if(!m_created) return(false);
      if(m_subscriber) eCAL_Sub_Destroy(m_subscriber);

      m_subscriber  = nullptr;
      m_topic_name  = "";
      m_topic_type  = "";
      m_created     = false;
      return(true);
    }

    bool SetQOS(const QOS::SReaderQOS& qos_)
    {
      // can not be set after Create
      if (m_subscriber) return(false);
      m_qos = qos_;
      return(true);
    }

    QOS::SReaderQOS GetQOS()
    {
      return(m_qos);
    }

    size_t Receive(std::string& buf_, long long* time_ = nullptr, int rcv_timeout_ = 0) const
    {
      if(!m_subscriber) return(0);
      void* buf = nullptr;
      size_t buf_len = eCAL_Sub_Receive_Alloc(m_subscriber, &buf, time_, rcv_timeout_);
      if(buf_len > 0)
      {
        buf_ = std::string(static_cast<char*>(buf), buf_len);
        eCAL_FreeMem(buf);
      }
      else
      {
        buf_.clear();
      }
      return(buf_.size());
    }

    static void ReceiveCallback(const char* topic_name_, const struct SReceiveCallbackDataC* data_, void* par_)
    {
      if(par_ == nullptr) return;
      CSubscriber* sub = static_cast<CSubscriber*>(par_);
      SReceiveCallbackData data;
      data.buf   = data_->buf;
      data.size  = data_->size;
      data.id    = data_->id;
      data.time  = data_->time;
      data.clock = data_->clock;
      sub->m_receive_callback(topic_name_, &data);
    }
    bool AddReceiveCallback(ReceiveCallbackT callback_)
    {
      if(!m_subscriber) return(false);
      m_receive_callback = callback_;
      return(eCAL_Sub_AddReceiveCallbackC(m_subscriber, &CSubscriber::ReceiveCallback, this) != 0);
    }

    bool RemReceiveCallback()
    {
      if(!m_subscriber) return(false);
      return(eCAL_Sub_RemReceiveCallback(m_subscriber) != 0);
    }

    static void SubEventCallback(const char* topic_name_, const struct SSubEventCallbackDataC* data_, void* par_)
    {
      if (par_ == nullptr) return;
      CSubscriber* sub = static_cast<CSubscriber*>(par_);
      SSubEventCallbackData data;
      data.type = data_->type;
      data.time = data_->time;
      sub->m_event_callback(topic_name_, &data);
    }
    bool AddEventCallback(eCAL_Subscriber_Event type_, SubEventCallbackT callback_)
    {
      if (!m_subscriber) return(false);
      m_event_callback = callback_;
      return(eCAL_Sub_AddEventCallbackC(m_subscriber, type_, &CSubscriber::SubEventCallback, this) != 0);
    }

    bool RemEventCallback(eCAL_Subscriber_Event type_)
    {
      if (!m_subscriber) return(false);
      return(eCAL_Sub_RemEventCallback(m_subscriber, type_) != 0);
    }

    bool IsCreated() const {return(m_created);}

    const std::string GetTopicName() const {return(m_topic_name);}

    const std::string GetTypeName() const {return(m_topic_type);}

    std::string GetDescription() const
    {
      if(!m_subscriber) return("");
      std::string desc;
      void* buf = nullptr;
      size_t buf_len = eCAL_Sub_GetDescription(m_subscriber, &buf, ECAL_ALLOCATE_4ME);
      if(buf_len > 0)
      {
        desc = std::string(static_cast<char*>(buf), buf_len);
        eCAL_FreeMem(buf);
      }
      return(desc);
    }

    bool SetTimeout(int timeout_)
    {
      if (!m_subscriber) return(false);
      return(eCAL_Sub_SetTimeout(m_subscriber, timeout_) != 0);
    }

    void InitializeQOS()
    {
      m_qos.history_kind       = QOS::keep_last_history_qos;
      m_qos.history_kind_depth = 8;
      m_qos.reliability        = QOS::reliable_reliability_qos;
    }

    std::string Dump() const
    {
      if(!m_subscriber) return("");
      void* buf = nullptr;
      size_t buf_len = eCAL_Sub_Dump(m_subscriber, &buf, ECAL_ALLOCATE_4ME);
      if(buf_len == 0) return("");
      std::string dbuf = std::string(static_cast<char*>(buf), buf_len);
      eCAL_FreeMem(buf);
      return(dbuf);
    }

  protected:
    ECAL_HANDLE       m_subscriber;
    std::string       m_topic_name;
    std::string       m_topic_type;
    std::string       m_topic_desc;
    QOS::SReaderQOS   m_qos;
    ReceiveCallbackT  m_receive_callback;
    SubEventCallbackT m_event_callback;
    bool              m_created;

#if defined(_MSC_VER) && (_MSC_VER < 1900) // VS2015 and newer
  private:
    // this object must not be copied.
    CSubscriber(const CSubscriber&);
    CSubscriber& operator=(const CSubscriber&);
#endif /* defined(_MSC_VER) && (_MSC_VER <= 1900) */

  };
};

#endif /* ! ECAL_C_DLL */
