/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
  class [[deprecated("Please use CMessageSubscriber instead")]] CMsgSubscriber : public CSubscriber
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
     * @param topic_info_  Topic type information (encoding, type, descriptor).
    **/
    CMsgSubscriber(const std::string& topic_name_, const struct SDataTypeInformation& topic_info_) : CSubscriber(topic_name_, topic_info_)
    {
    }

    virtual ~CMsgSubscriber() = default;

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
    CMsgSubscriber(CMsgSubscriber&& rhs) noexcept
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
    CMsgSubscriber& operator=(CMsgSubscriber&& rhs) noexcept
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

    /**
     * @brief Creates this object.
     *
     * @param topic_name_   Unique topic name.
     * @param topic_info_  Topic type information (encoding, type, descriptor).
     *
     * @return  true if it succeeds, false if it fails.
    **/
    bool Create(const std::string& topic_name_, const struct SDataTypeInformation& topic_info_)
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
    using MsgReceiveCallbackT = std::function<void (const char *, const T &, long long, long long, long long)>;

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

      {
        std::lock_guard<std::mutex> callback_lock(m_cb_callback_mutex);
        m_cb_callback = callback_;
      }
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
      bool ret = CSubscriber::RemReceiveCallback();

      std::lock_guard<std::mutex> callback_lock(m_cb_callback_mutex);
      if (m_cb_callback == nullptr) return(false);
      m_cb_callback = nullptr;
      return(ret);
    }

protected:
    // We cannot make it pure virtual, as it would break a bunch of implementations, who are not (yet) implementing this function
    virtual SDataTypeInformation GetDataTypeInformation() const { return SDataTypeInformation{}; }
    virtual bool Deserialize(T& msg_, const void* buffer_, size_t size_) const = 0;

  private:
    void ReceiveCallback(const char* topic_name_, const struct eCAL::SReceiveCallbackData* data_)
    {
      MsgReceiveCallbackT fn_callback(nullptr);
      {
        std::lock_guard<std::mutex> callback_lock(m_cb_callback_mutex);
        fn_callback = m_cb_callback;
      }

      if(fn_callback == nullptr) return;

      T msg;
      if(Deserialize(msg, data_->buf, data_->size))
      {
        (fn_callback)(topic_name_, msg, data_->time, data_->clock, data_->id);
      }
    }

    std::mutex          m_cb_callback_mutex;
    MsgReceiveCallbackT m_cb_callback;
  };

  /**
   * @brief  eCAL abstract message subscriber class.
   *
   * Abstract subscriber template class for messages.
   * This class has two template arguments, the actual type and a deserializer class.
   * The deserializer class is responsible for providing datatype information, and providing a method to convert from `void*` to `T`.
   * This allows to specify classes with common deserializers, e.g. like a ProtobufMessageSubscriber, StringMessageSubscriber etc.
   *
  **/
  template <typename T, typename Deserializer>
  class CMessageSubscriber final : public CSubscriber
  {
  public:
    /**
     * @brief  Constructor.
    **/
    CMessageSubscriber() : CSubscriber()
    {
    }

    /**
    * @brief  Constructor.
    *
    * @param topic_name_  Unique topic name.
    * @param config_      Optional configuration parameters.
    **/
    CMessageSubscriber(const std::string& topic_name_, const Subscriber::Configuration& config_ = GetSubscriberConfiguration()) : CSubscriber()
      , m_deserializer()
    {
      SDataTypeInformation topic_info = m_deserializer.GetDataTypeInformation();
      CSubscriber::Create(topic_name_, topic_info, config_);
    }

    ~CMessageSubscriber() noexcept
    {
      Destroy();
    };

    /**
    * @brief  Copy Constructor is not available.
    **/
    CMessageSubscriber(const CMessageSubscriber&) = delete;

    /**
    * @brief  Copy Constructor is not available.
    **/
    CMessageSubscriber& operator=(const CMessageSubscriber&) = delete;

    /**
    * @brief  Move Constructor
    **/
    CMessageSubscriber(CMessageSubscriber&& rhs)
      : CSubscriber(std::move(rhs))
      , m_cb_callback(std::move(rhs.m_cb_callback))
      , m_deserializer(std::move(rhs.m_deserializer))
    {
      bool has_callback = (m_cb_callback != nullptr);

      if (has_callback)
      {
        // the callback bound to the CSubscriber belongs to rhs, bind to this callback instead
        CSubscriber::RemReceiveCallback();
        auto callback = std::bind(&CMessageSubscriber::ReceiveCallback, this, std::placeholders::_1, std::placeholders::_2);
        CSubscriber::AddReceiveCallback(callback);
      }
    }

    /**
    * @brief  Move assignment
    **/
    CMessageSubscriber& operator=(CMessageSubscriber&& rhs)
    {
      Destroy();

      CSubscriber::operator=(std::move(rhs));

      m_cb_callback = std::move(rhs.m_cb_callback);
      m_deserializer = std::move(rhs.m_deserializer);
        
      bool has_callback(m_cb_callback != nullptr);

      if (has_callback)
      {
        // the callback bound to the CSubscriber belongs to rhs, bind to this callback instead;
        CSubscriber::RemReceiveCallback();
        auto callback = std::bind(&CMessageSubscriber::ReceiveCallback, this, std::placeholders::_1, std::placeholders::_2);
        CSubscriber::AddReceiveCallback(callback);
      }

      return *this;
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
    bool Receive(T& msg_, long long* time_ = nullptr, int rcv_timeout_ = 0)
    {
      std::string rec_buf;
      bool success = CSubscriber::ReceiveBuffer(rec_buf, time_, rcv_timeout_);
      if (!success) return(false);
      // In the future, I would like to get m_datatype_info from the ReceiveBuffer fuunction!
      return(m_deserializer.Deserialize(msg_, rec_buf.c_str(), rec_buf.size()));
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
      RemReceiveCallback();

      {
        std::lock_guard<std::mutex> callback_lock(m_cb_callback_mutex);
        m_cb_callback = callback_;
      }
      auto callback = std::bind(&CMessageSubscriber::ReceiveCallback, this, std::placeholders::_1, std::placeholders::_2);
      return(CSubscriber::AddReceiveCallback(callback));
    }

    /**
     * @brief  Remove receive callback for incoming messages.
     *
     * @return  True if it succeeds, false if it fails.
    **/
    bool RemReceiveCallback()
    {
      bool ret = CSubscriber::RemReceiveCallback();

      std::lock_guard<std::mutex> callback_lock(m_cb_callback_mutex);
      if (m_cb_callback == nullptr) return(false);
      m_cb_callback = nullptr;
      return(ret);
    }

  private:
    void ReceiveCallback(const char* topic_name_, const struct eCAL::SReceiveCallbackData* data_)
    {
      MsgReceiveCallbackT fn_callback = nullptr;
      {
        std::lock_guard<std::mutex> callback_lock(m_cb_callback_mutex);
        fn_callback = m_cb_callback;
      }

      if (fn_callback == nullptr) return;

      T msg;
      // In the future, I would like to get m_datatype_info from the ReceiveBuffer function!
      if (m_deserializer.Deserialize(msg, data_->buf, data_->size))
      {
        (fn_callback)(topic_name_, msg, data_->time, data_->clock, data_->id);
      }
    }

    std::mutex           m_cb_callback_mutex;
    MsgReceiveCallbackT  m_cb_callback;
    Deserializer         m_deserializer;
  };
}
