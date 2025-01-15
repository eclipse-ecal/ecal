
.. _program_listing_file_ecal_core_include_ecal_msg_subscriber.h:

Program Listing for File subscriber.h
=====================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_msg_subscriber.h>` (``ecal/core/include/ecal/msg/subscriber.h``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

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
     template <typename T>
     class CMsgSubscriber : public CSubscriber
     {
     public:
       CMsgSubscriber() : CSubscriber()
       {
       }
   
       ECAL_DEPRECATE_SINCE_5_13("Please use the constructor CMsgSubscriber(const std::string& topic_name_, const SDataTypeInformation& topic_info_) instead. This function will be removed in future eCAL versions.")
       CMsgSubscriber(const std::string& topic_name_, const std::string& topic_type_ = "", const std::string& topic_desc_ = "") : CSubscriber(topic_name_, topic_type_, topic_desc_)
       {
       }
   
       CMsgSubscriber(const std::string& topic_name_, const SDataTypeInformation& topic_info_) : CSubscriber(topic_name_, topic_info_)
       {
       }
   
       virtual ~CMsgSubscriber() = default;
   
       CMsgSubscriber(const CMsgSubscriber&) = delete;
   
       CMsgSubscriber& operator=(const CMsgSubscriber&) = delete;
   
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
   
       ECAL_DEPRECATE_SINCE_5_13("Please use the method CMsgSubscriber(const std::string& topic_name_, const SDataTypeInformation& topic_info_) instead. This function will be removed in future eCAL versions.")
       bool Create(const std::string& topic_name_, const std::string& topic_type_ = "", const std::string& topic_desc_ = "")
       {
         return(CSubscriber::Create(topic_name_, topic_type_, topic_desc_));
       }
   
       bool Create(const std::string& topic_name_, const SDataTypeInformation& topic_info_)
       {
         return(CSubscriber::Create(topic_name_, topic_info_));
       }
   
       bool Destroy()
       {
         RemReceiveCallback();
         return(CSubscriber::Destroy());
       }
   
       bool Receive(T& msg_, long long* time_ = nullptr, int rcv_timeout_ = 0) const
       {
         assert(IsCreated());
         std::string rec_buf;
         bool success = CSubscriber::ReceiveBuffer(rec_buf, time_, rcv_timeout_);
         if (!success) return(false);
         return(Deserialize(msg_, rec_buf.c_str(), rec_buf.size()));
       }
   
       typedef std::function<void(const char* topic_name_, const T& msg_, long long time_, long long clock_, long long id_)> MsgReceiveCallbackT;
   
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
   
       bool RemReceiveCallback()
       {
         bool ret = CSubscriber::RemReceiveCallback();
   
         std::lock_guard<std::mutex> callback_lock(m_cb_callback_mutex);
         if (m_cb_callback == nullptr) return(false);
         m_cb_callback = nullptr;
         return(ret);
       }
   
   protected:
       ECAL_DEPRECATE_SINCE_5_13("Please use SDataTypeInformation GetDataTypeInformation() instead. This function will be removed in future eCAL versions.")
       virtual std::string GetTypeName() const
       {
         SDataTypeInformation topic_info{ GetDataTypeInformation() };
         return Util::CombinedTopicEncodingAndType(topic_info.encoding, topic_info.name);
       };
   
       ECAL_DEPRECATE_SINCE_5_13("Please use SDataTypeInformation GetDataTypeInformation() instead. This function will be removed in future eCAL versions.")
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
         MsgReceiveCallbackT fn_callback = nullptr;
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
   }
