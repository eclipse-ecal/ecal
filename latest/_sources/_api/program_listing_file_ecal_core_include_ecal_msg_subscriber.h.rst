
.. _program_listing_file_ecal_core_include_ecal_msg_subscriber.h:

Program Listing for File subscriber.h
=====================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_msg_subscriber.h>` (``ecal/core/include/ecal/msg/subscriber.h``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

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
   
   #pragma once
   
   #include <ecal/deprecate.h>
   #include <ecal/pubsub/subscriber.h>
   #include <ecal/util.h>
   
   #include <cassert>
   #include <cstring>
   #include <functional>
   #include <mutex>
   #include <string>
   #include <vector>
   
   namespace eCAL
   {
     template <typename T, typename Deserializer>
     class CMessageSubscriber final : public CSubscriber
     {
     public:
       explicit CMessageSubscriber(const std::string& topic_name_, const Subscriber::Configuration& config_ = GetSubscriberConfiguration()) : CSubscriber(topic_name_, m_deserializer.GetDataTypeInformation(), config_)
         , m_deserializer()
       {
       }
   
       explicit CMessageSubscriber(const std::string& topic_name_, const SubEventCallbackT& event_callback_, const Subscriber::Configuration& config_ = GetSubscriberConfiguration()) : CSubscriber(topic_name_, m_deserializer.GetDataTypeInformation(), event_callback_, config_)
         , m_deserializer()
       {
       }
   
       ~CMessageSubscriber() noexcept
       {
       };
   
       CMessageSubscriber(const CMessageSubscriber&) = delete;
   
       CMessageSubscriber& operator=(const CMessageSubscriber&) = delete;
   
       CMessageSubscriber(CMessageSubscriber&& rhs)
         : CSubscriber(std::move(rhs))
         , m_cb_callback(std::move(rhs.m_cb_callback))
         , m_deserializer(std::move(rhs.m_deserializer))
       {
         bool has_callback = (m_cb_callback != nullptr);
   
         if (has_callback)
         {
           // the callback bound to the CSubscriber belongs to rhs, bind to this callback instead
           CSubscriber::RemoveReceiveCallback();
           auto callback = std::bind(&CMessageSubscriber::ReceiveCallback, this, std::placeholders::_1, std::placeholders::_3);
           CSubscriber::SetReceiveCallback(callback);
         }
       }
   
       CMessageSubscriber& operator=(CMessageSubscriber&& rhs)
       {
         CSubscriber::operator=(std::move(rhs));
   
         m_cb_callback = std::move(rhs.m_cb_callback);
         m_deserializer = std::move(rhs.m_deserializer);
           
         bool has_callback(m_cb_callback != nullptr);
   
         if (has_callback)
         {
           // the callback bound to the CSubscriber belongs to rhs, bind to this callback instead;
           CSubscriber::RemoveReceiveCallback();
           auto callback = std::bind(&CMessageSubscriber::ReceiveCallback, this, std::placeholders::_1, std::placeholders::_3);
           CSubscriber::SetReceiveCallback(callback);
         }
   
         return *this;
       }
   
       using MsgReceiveCallbackT = std::function<void(const STopicId& topic_id_, const T& msg_, long long time_, long long clock_, long long id_)>;
   
       bool SetReceiveCallback(MsgReceiveCallbackT callback_)
       {
         RemoveReceiveCallback();
   
         {
           std::lock_guard<std::mutex> callback_lock(m_cb_callback_mutex);
           m_cb_callback = callback_;
         }
         auto callback = std::bind(&CMessageSubscriber::ReceiveCallback, this, std::placeholders::_1, std::placeholders::_3);
         return(CSubscriber::SetReceiveCallback(callback));
       }
   
       bool RemoveReceiveCallback()
       {
         bool ret = CSubscriber::RemoveReceiveCallback();
   
         std::lock_guard<std::mutex> callback_lock(m_cb_callback_mutex);
         if (m_cb_callback == nullptr) return(false);
         m_cb_callback = nullptr;
         return(ret);
       }
   
     private:
       void ReceiveCallback(const STopicId& topic_id_, const SReceiveCallbackData& data_)
       {
         MsgReceiveCallbackT fn_callback = nullptr;
         {
           std::lock_guard<std::mutex> callback_lock(m_cb_callback_mutex);
           fn_callback = m_cb_callback;
         }
   
         if (fn_callback == nullptr) return;
   
         T msg;
         // In the future, I would like to get m_datatype_info from the ReceiveBuffer function!
         if (m_deserializer.Deserialize(msg, data_.buf, data_.size))
         {
           (fn_callback)(topic_id_, msg, data_.time, data_.clock, data_.id);
         }
       }
   
       std::mutex           m_cb_callback_mutex;
       MsgReceiveCallbackT  m_cb_callback;
       Deserializer         m_deserializer;
     };
   }
