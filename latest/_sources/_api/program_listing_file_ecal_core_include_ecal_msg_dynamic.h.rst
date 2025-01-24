
.. _program_listing_file_ecal_core_include_ecal_msg_dynamic.h:

Program Listing for File dynamic.h
==================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_msg_dynamic.h>` (``ecal/core/include/ecal/msg/dynamic.h``)

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
   
   #include <exception>
   #include <optional>
   #include <mutex>
   #include <string>
   
   #include <ecal/v5/ecal_subscriber.h>
   #include <ecal/util.h>
   
   namespace eCAL
   {
     /* @cond */
     class DynamicReflectionException : public std::exception
     {
     public:
       DynamicReflectionException(const std::string& message) : message_(message) {}
       virtual const char* what() const noexcept { return message_.c_str(); }
     private:
       std::string message_;
     };
     /* @endcond */
   
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
     class CDynamicMessageSubscriber final : public v5::CSubscriber
     {
     public:
       CDynamicMessageSubscriber() = default;
   
       CDynamicMessageSubscriber(const std::string& topic_name_) : v5::CSubscriber()
         , m_cb_callback(nullptr)
         , m_error_callback(nullptr)
         , m_deserializer()
       {
         v5::CSubscriber::Create(topic_name_);
       }
   
       ~CDynamicMessageSubscriber() noexcept
       {
         Destroy();
       };
   
       CDynamicMessageSubscriber(const CDynamicMessageSubscriber&) = delete;
   
       CDynamicMessageSubscriber& operator=(const CDynamicMessageSubscriber&) = delete;
   
       CDynamicMessageSubscriber(CDynamicMessageSubscriber&& rhs)
         : v5::CSubscriber(std::move(rhs))
         , m_cb_callback(std::move(rhs.m_cb_callback))
         , m_deserializer(std::move(rhs.m_deserializer))
       {
         bool has_callback = (m_cb_callback != nullptr);
   
         if (has_callback)
         {
           // the callback bound to the CSubscriber belongs to rhs, bind to this callback instead
           v5::CSubscriber::RemReceiveCallback();
           auto callback = std::bind(&CDynamicMessageSubscriber::ReceiveCallback, this, std::placeholders::_1, std::placeholders::_2);
           v5::CSubscriber::AddReceiveCallback(callback);
         }
       }
   
       CDynamicMessageSubscriber& operator=(CDynamicMessageSubscriber&& rhs) = delete;
   
       bool Destroy()
       {
         RemReceiveCallback();
         return(v5::CSubscriber::Destroy());
       }
   
       using MsgReceiveCallbackT = std::function<void(const STopicId& topic_id_, const T& msg_, long long time_, long long clock_, long long id_)>;
   
       bool AddReceiveCallback(MsgReceiveCallbackT callback_)
       {
         RemReceiveCallback();
   
         {
           std::lock_guard<std::mutex> callback_lock(m_cb_callback_mutex);
           m_cb_callback = callback_;
         }
   
         ReceiveCallbackT callback = std::bind(&CDynamicMessageSubscriber::ReceiveCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
         return(v5::CSubscriber::AddReceiveCallback(callback));
       }
   
       bool RemReceiveCallback()
       {
         bool ret = v5::CSubscriber::RemReceiveCallback();
   
         std::lock_guard<std::mutex> callback_lock(m_cb_callback_mutex);
         if (m_cb_callback == nullptr) return(false);
         m_cb_callback = nullptr;
         return(ret);
       }
   
       using ErrorCallbackT = std::function<void(const std::string& error)>;
   
       bool AddErrorCallback(ErrorCallbackT callback_)
       {
         std::lock_guard<std::mutex> callback_lock(m_error_callback_mutex);
         m_error_callback = callback_;
   
         return true;
       }
       
       bool RemErrorCallback()
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
           std::lock_guard<std::mutex> callback_lock(m_cb_callback_mutex);
           fn_callback = m_cb_callback;
         }
   
         if (fn_callback == nullptr) return;
   
         try
         {
           auto msg = m_deserializer.Deserialize(data_.buf, data_.size, topic_info_);
           fn_callback(topic_id_, msg, data_.time, data_.clock, data_.id);
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
   
       std::mutex           m_cb_callback_mutex;
       MsgReceiveCallbackT  m_cb_callback;
       std::mutex           m_error_callback_mutex;
       ErrorCallbackT       m_error_callback;
       DynamicDeserializer  m_deserializer;
     };
   
   }
