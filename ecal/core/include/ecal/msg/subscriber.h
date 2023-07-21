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

#include <ecal/ecal_deprecate.h>
#include <ecal/ecal_subscriber.h>
#include <ecal/ecal_util.h>

#include <cassert>
#include <cstring>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

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
    CMsgSubscriber() : CSubscriber()
    {
    }

    /**
     * @brief  Constructor.
     *
     * @param topic_name_  Unique topic name.
     * @param topic_type_  Type name (optional for type checking).
     * @param topic_desc_  Type description (optional for description checking).
    **/
    ECAL_DEPRECATE_SINCE_5_13("Please use the constructor CMsgSubscriber(const std::string& topic_name_, const SDataTypeInformation& topic_info_) instead. This function will be removed in eCAL6. ")
    CMsgSubscriber(const std::string& topic_name_, const std::string& topic_type_ = "", const std::string& topic_desc_ = "") : CSubscriber(topic_name_, topic_type_, topic_desc_)
    {
    }

    /**
    * @brief  Constructor.
    *
    * @param topic_name_  Unique topic name.
    * @param topic_info_  Topic type information (encoding, type, descriptor).
    **/
    CMsgSubscriber(const std::string& topic_name_, const SDataTypeInformation& topic_info_) : CSubscriber(topic_name_, topic_info_)
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
      , m_cb_callback(std::move(rhs.m_cb_callback))
    {
      bool has_callback = (m_cb_callback != nullptr);

      if (has_callback)
      {
        // the callback bound to the CSubscriber belongs to rhs, bind to this callback instead
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

      m_cb_callback = std::move(rhs.m_cb_callback);
      bool has_callback(m_cb_callback != nullptr);

      if (has_callback)
      {
        // the callback bound to the CSubscriber belongs to rhs, bind to this callback instead;
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
    ECAL_DEPRECATE_SINCE_5_13("Please use the method CMsgSubscriber(const std::string& topic_name_, const SDataTypeInformation& topic_info_) instead. This function will be removed in eCAL6. ")
    bool Create(const std::string& topic_name_, const std::string& topic_type_ = "", const std::string& topic_desc_ = "")
    {
      return(CSubscriber::Create(topic_name_, topic_type_, topic_desc_));
    }

    /**
    * @brief Creates this object.
    *
    * @param topic_name_   Unique topic name.
    * @param topic_info_  Topic type information (encoding, type, descriptor).
    *
    * @return  true if it succeeds, false if it fails.
    **/
    bool Create(const std::string& topic_name_, const SDataTypeInformation& topic_info_)
    {
      return(CSubscriber::Create(topic_name_, topic_info_));
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
      if (!success) return(false);
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
      if (m_cb_callback == nullptr) return(false);
      m_cb_callback = nullptr;
      return(CSubscriber::RemReceiveCallback());
    }

protected:
    ECAL_DEPRECATE_SINCE_5_13("Please use SDataTypeInformation GetDataTypeInformation() instead. This function will be removed in eCAL6.")
    virtual std::string GetTypeName() const
    {
      SDataTypeInformation topic_info{ GetDataTypeInformation() };
      return Util::CombinedTopicEncodingAndType(topic_info.encoding, topic_info.name);
    };

    ECAL_DEPRECATE_SINCE_5_13("Please use SDataTypeInformation GetDataTypeInformation() instead. This function will be removed in eCAL6.")
    virtual std::string GetDescription() const
    {
      return GetDataTypeInformation().descriptor;
    };

    // We cannot make it pure virtual, as it would break a bunch of implementations, who are not (yet) implementing this function
    virtual SDataTypeInformation GetDataTypeInformation() const { return SDataTypeInformation{}; }
    virtual bool Deserialize(T& msg_, const void* buffer_, size_t size_) const = 0;

  private:
    void ReceiveCallback(const char* topic_name_, const struct eCAL::SReceiveCallbackData* data_)
    {
      MsgReceiveCallbackT fn_callback(m_cb_callback);

      if(fn_callback == nullptr) return;

      T msg;
      if(Deserialize(msg, data_->buf, data_->size))
      {
        (fn_callback)(topic_name_, msg, data_->time, data_->clock, data_->id);
      }
    }

    MsgReceiveCallbackT m_cb_callback;
  };
}
