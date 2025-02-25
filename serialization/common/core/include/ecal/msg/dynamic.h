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
 * @file   msg/dynamic.h
 * @brief  eCAL dynamic reflection exception
**/

#pragma once

#include <exception>
#include <optional>
#include <mutex>
#include <string>

#include <ecal/pubsub/subscriber.h>
#include <ecal/util.h>

namespace eCAL
{
  /* @cond */
  inline bool StrEmptyOrNull(const std::string& str)
  {
    if (str.empty())
    {
      return true;
    }
    else
    {
      for (auto c : str)
      {
        if (c != '\0')
        {
          return false;
        }
      }
      return true;
    }
  }
  /* @endcond */


  template <typename T, typename DynamicDeserializer>
  class CDynamicMessageSubscriber
  {
  public:
    /**
     * @param topic_name_      Unique topic name.
     * @param event_callback_  Callback for subscriber events.
     * @param config_          Configuration parameters.
    **/
    CDynamicMessageSubscriber(const std::string& topic_name_, const Subscriber::Configuration& config_ = GetSubscriberConfiguration())
      : m_deserializer()
      , m_subscriber(topic_name_, m_deserializer.GetDataTypeInformation(), config_)
      , m_data_callback(nullptr)
      , m_error_callback(nullptr)
    {
    }

    /**
     * @param topic_name_      Unique topic name.
     * @param event_callback_  Callback for subscriber events.
     * @param config_          Configuration parameters.
    **/
    CDynamicMessageSubscriber(const std::string& topic_name_, const SubEventCallbackT& event_callback_, const Subscriber::Configuration& config_ = GetSubscriberConfiguration())
      : m_deserializer()
      , m_subscriber(topic_name_, m_deserializer.GetDataTypeInformation(), event_callback_, config_)
      , m_data_callback(nullptr)
      , m_error_callback(nullptr)
    {
    }


    ~CDynamicMessageSubscriber() noexcept
    {
      RemoveReceiveCallback();
    };

    /**
    * @brief Copy Constructor is not available.
    **/
    CDynamicMessageSubscriber(const CDynamicMessageSubscriber&) = delete;

    /**
    * @brief Copy Constructor is not available.
    **/
    CDynamicMessageSubscriber& operator=(const CDynamicMessageSubscriber&) = delete;

    /**
    * @brief Move Constructor
    **/
    CDynamicMessageSubscriber(CDynamicMessageSubscriber&& rhs)
      : m_deserializer(std::move(rhs.m_deserializer))
      , m_subscriber(std::move(rhs.subscriber))
      , m_cb_callback(std::move(rhs.m_cb_callback))
      , m_error_callback(std::move(rhs.m_error_callback))
    {
      bool has_callback = (m_cb_callback != nullptr);

      if (has_callback)
      {
        // the callback bound to the CSubscriber belongs to rhs, bind to this callback instead
        CSubscriber::RemoveReceiveCallback();
        auto callback = std::bind(&CDynamicMessageSubscriber::ReceiveCallback, this, std::placeholders::_1, std::placeholders::_2);
        CSubscriber::SetReceiveCallback(callback);
      }
    }

    /**
    * @brief Move assignment not available
    **/
    CDynamicMessageSubscriber& operator=(CDynamicMessageSubscriber&& rhs) = delete;

    /**
     * @brief eCAL message receive callback function
     *
     * @param topic_id_  Topic id of the data source (publisher).
     * @param msg_       Message content.
     * @param time_      Message time stamp.
     * @param clock_     Message writer clock.
     **/
    using MsgReceiveCallbackT = std::function<void(const STopicId& topic_id_, const T& msg_, long long time_, long long clock_)>;

    /**
     * @brief Set receive callback for incoming messages.
     *
     * @param callback_  The callback function.
     *
     * @return  True if it succeeds, false if it fails.
    **/
    bool SetReceiveCallback(MsgReceiveCallbackT callback_)
    {
      RemoveReceiveCallback();

      {
        std::lock_guard<std::mutex> callback_lock(m_data_callback_mutex);
        m_data_callback = callback_;
      }

      ReceiveCallbackT callback = std::bind(&CDynamicMessageSubscriber::ReceiveCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
      return(m_subscriber.SetReceiveCallback(callback));
    }

    /**
     * @brief Remove receive callback for incoming messages.
     *
     * @return  True if it succeeds, false if it fails.
    **/
    bool RemoveReceiveCallback()
    {
      bool ret = m_subscriber.RemoveReceiveCallback();

      std::lock_guard<std::mutex> callback_lock(m_data_callback_mutex);
      if (m_data_callback == nullptr) return(false);
      m_data_callback = nullptr;
      return(ret);
    }

    /**
     * @brief Callback function in case an error occurs.
     *
     * @param error  The error message string.
    **/
    using ErrorCallbackT = std::function<void(const std::string& error)>;

    /**
     * @brief Set callback function in case an error occurs.
     *
     * @param callback_  The callback function to add.
     *
     * @return  True if succeeded, false if not.
    **/
    bool SetErrorCallback(ErrorCallbackT callback_)
    {
      std::lock_guard<std::mutex> callback_lock(m_error_callback_mutex);
      m_error_callback = callback_;

      return true;
    }
    
    /**
     * @brief Remove callback function in case an error occurs.
     *
     * @return  True if succeeded, false if not.
    **/
    bool RemoveErrorCallback()
    {
      std::lock_guard<std::mutex> callback_lock(m_error_callback_mutex);
      m_error_callback = nullptr;

      return true;
    }

  private:
    void ReceiveCallback(const STopicId& topic_id_, const SDataTypeInformation& topic_info_, const struct SReceiveCallbackData& data_)
    {
      MsgReceiveCallbackT fn_callback = nullptr;
      {
        std::lock_guard<std::mutex> callback_lock(m_data_callback_mutex);
        fn_callback = m_data_callback;
      }

      if (fn_callback == nullptr) return;

      try
      {
        auto msg = m_deserializer.Deserialize(data_.buffer, data_.buffer_size, topic_info_);
        fn_callback(topic_id_, msg, data_.send_timestamp, data_.send_clock);
      }
      catch (const DynamicReflectionException& e)
      {
        CallErrorCallback(std::string("Dynamic Deserialization: Error deserializing data: ") + e.what() );
      }
    }

    void CallErrorCallback(const std::string& message)
    {
      ErrorCallbackT error_callback = nullptr;
      {
        std::lock_guard<std::mutex> callback_lock(m_error_callback_mutex);
        error_callback = m_error_callback;
      }
      if (error_callback)
      {
        error_callback(message);
      }
    }

    //It's important that m_deserializer is created before m_suubscriber, because it needs to retrieve the DatatypeInformation
    DynamicDeserializer  m_deserializer;
    CSubscriber          m_subscriber;

    std::mutex           m_data_callback_mutex;
    MsgReceiveCallbackT  m_data_callback;
    std::mutex           m_error_callback_mutex;
    ErrorCallbackT       m_error_callback;
  };

}
