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
 * @brief  eCAL message subscriber interface
**/

#pragma once

#include <ecal/ecal_publisher.h>
#include <ecal/ecal_subscriber.h>

#include <string>
#include <vector>
#include <functional>
#include <assert.h>
#include <string.h>

namespace eCAL
{
  /**
   * @brief  eCAL abstract message subscriber class.
   *
   * Abstract subscriber template class for messages. For details see documentation of CPublisher class.
   * 
  **/
  template <typename T>
  class CMsgSubscriber : public CSubscriber
  {
  public:
    /**
     * @brief  Constructor. 
    **/
    CMsgSubscriber() : CSubscriber() , m_cb_active(false)
    {
    }

    /**
     * @brief  Constructor. 
     *
     * @param topic_name_  Unique topic name. 
     * @param topic_type_  Type name (optional for type checking). 
     * @param topic_desc_  Type description (optional for description checking).
    **/
    CMsgSubscriber(const std::string& topic_name_, const std::string& topic_type_ = "", const std::string& topic_desc_ = "") : CSubscriber(topic_name_, topic_type_, topic_desc_ ), m_cb_active(false)
    {
    }

    /**
    * @brief  Copy Constructor is not available.
    **/
    CMsgSubscriber(const CMsgSubscriber&) = delete;

    /**
    * @brief  Copy Constructor is not available.
    **/
    CMsgSubscriber& operator=(const CMsgSubscriber&) = delete;

    /**
    * @brief  Move Constructor
    **/
    CMsgSubscriber(CMsgSubscriber&& rhs)
      : CSubscriber(std::move(rhs))
      , m_cb_active(rhs.m_cb_active)
      , m_cb_callback(std::move(rhs.m_cb_callback))
    {
      //m_cb_active = rhs.m_cb_active;
      rhs.m_cb_active = false;

      if (m_cb_active)
      {
        // The callback bound to the CSubscriber belongs to rhs, bind to this callback instead;
        CSubscriber::RemReceiveCallback();
        auto callback = std::bind(&CMsgSubscriber::ReceiveCallback, this, std::placeholders::_1, std::placeholders::_2);
        CSubscriber::AddReceiveCallback(callback);
      }
    }

    /**
    * @brief  Move assignment
    **/
    CMsgSubscriber& operator=(CMsgSubscriber&& rhs)
    {
      CSubscriber::operator=(std::move(rhs));

      m_cb_active = rhs.m_cb_active;
      rhs.m_cb_active = false;

      m_cb_callback = std::move(rhs.m_cb_callback);

      if (m_cb_active)
      {
        // The callback bound to the CSubscriber belongs to rhs, bind to this callback instead;
        CSubscriber::RemReceiveCallback();
        auto callback = std::bind(&CMsgSubscriber::ReceiveCallback, this, std::placeholders::_1, std::placeholders::_2);
        CSubscriber::AddReceiveCallback(callback);
      }

      return *this;
    }

    virtual ~CMsgSubscriber() {}

    /**
     * @brief Creates this object. 
     *
     * @param topic_name_   Unique topic name. 
     * @param topic_type_   Type name (optional for type checking). 
     * @param topic_desc_   Type description (optional for description checking).
     *
     * @return  true if it succeeds, false if it fails. 
    **/
    bool Create(const std::string& topic_name_, const std::string& topic_type_ = "", const std::string& topic_desc_ = "")
    {
      return(CSubscriber::Create(topic_name_, topic_type_, topic_desc_));
    }

    /**
     * @brief Destroys this object. 
     *
     * @return  true if it succeeds, false if it fails. 
    **/
    bool Destroy()
    {
      RemReceiveCallback();
      return(CSubscriber::Destroy());
    }

    /**
     * @brief  Receive deserialized message. 
     *
     * @param [out] msg_    The message object. 
     * @param [out] time_   Optional receive time stamp. 
     * @param rcv_timeout_  Receive timeout in ms. 
     *
     * @return  True if a message could received, false otherwise. 
    **/
    bool Receive(T& msg_, long long* time_ = nullptr, int rcv_timeout_ = 0) const
    {
      assert(IsCreated());
      std::string rec_buf;
      bool success = CSubscriber::ReceiveBuffer(rec_buf, time_, rcv_timeout_);
      if(!success) return(false);
      return(Deserialize(msg_, rec_buf.c_str(), rec_buf.size()));
    }

    /**
     * @brief eCAL message receive callback function
     *
     * @param topic_name_  Topic name of the data source (publisher).
     * @param msg_         Message content.
     * @param time_        Message time stamp.
     * @param clock_       Message writer clock.
     * @param id_          Message id.
     **/
    typedef std::function<void(const char* topic_name_, const T& msg_, long long time_, long long clock_, long long id_)> MsgReceiveCallbackT;

    /**
     * @brief  Add receive callback for incoming messages. 
     *
     * @param callback_  The callback function. 
     *
     * @return  True if it succeeds, false if it fails. 
    **/
    bool AddReceiveCallback(MsgReceiveCallbackT callback_)
    {
      assert(IsCreated());
      RemReceiveCallback();
      if(m_cb_active == true) return(false);
      m_cb_active   = true;
      m_cb_callback = callback_;
      auto callback = std::bind(&CMsgSubscriber::ReceiveCallback, this, std::placeholders::_1, std::placeholders::_2);
      return(CSubscriber::AddReceiveCallback(callback));
    }

    /**
     * @brief  Remove receive callback for incoming messages. 
     *
     * @return  True if it succeeds, false if it fails. 
    **/
    bool RemReceiveCallback()
    {
      if(m_cb_active == false) return(false);
      m_cb_active = false;
      return(CSubscriber::RemReceiveCallback());
    }

  private:
    virtual std::string GetTypeName() const = 0;
    virtual std::string GetDescription() const = 0;
    virtual bool Deserialize(T& msg_, const void* buffer_, size_t size_) const = 0;

    void ReceiveCallback(const char* topic_name_, const struct eCAL::SReceiveCallbackData* data_)
    {
      if(m_cb_active == false) return;
      assert(m_cb_callback != nullptr);

      T msg;
      if(Deserialize(msg, data_->buf, data_->size))
      {
        if(m_cb_callback != nullptr)
        {
          (m_cb_callback)(topic_name_, msg, data_->time, data_->clock, data_->id);
        }
      }
    }

    bool                 m_cb_active;
    MsgReceiveCallbackT  m_cb_callback;
  };
}
