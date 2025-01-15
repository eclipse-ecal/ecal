
.. _program_listing_file_ecal_core_include_ecal_ecal_subscriber_v5.h:

Program Listing for File ecal_subscriber_v5.h
=============================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_subscriber_v5.h>` (``ecal/core/include/ecal/ecal_subscriber_v5.h``)

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
   
   #include <cstddef>
   #include <ecal/ecal_callback.h>
   #include <ecal/ecal_callback_v5.h>
   #include <ecal/ecal_deprecate.h>
   #include <ecal/ecal_os.h>
   #include <ecal/ecal_config.h>
   #include <ecal/ecal_types.h>
   
   #include <memory>
   #include <set>
   #include <string>
   
   namespace eCAL
   {
     class CSubscriberImpl;
   
     namespace v5
     {
       class ECAL_API_CLASS CSubscriber
       {
       public:
         ECAL_API_EXPORTED_MEMBER
           CSubscriber();
   
         ECAL_API_EXPORTED_MEMBER
           CSubscriber(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const Subscriber::Configuration& config_ = GetSubscriberConfiguration());
   
         ECAL_API_EXPORTED_MEMBER
           explicit CSubscriber(const std::string& topic_name_, const Subscriber::Configuration& config_ = GetSubscriberConfiguration());
   
         ECAL_API_EXPORTED_MEMBER
           virtual ~CSubscriber();
   
         CSubscriber(const CSubscriber&) = delete;
   
         CSubscriber& operator=(const CSubscriber&) = delete;
   
         ECAL_API_EXPORTED_MEMBER
           CSubscriber(CSubscriber&& rhs) noexcept;
   
         ECAL_API_EXPORTED_MEMBER
           CSubscriber& operator=(CSubscriber&& rhs) noexcept;
   
         ECAL_API_EXPORTED_MEMBER
           bool Create(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const Subscriber::Configuration& config_ = GetSubscriberConfiguration());
   
         ECAL_API_EXPORTED_MEMBER
           bool Create(const std::string& topic_name_);
   
         ECAL_API_EXPORTED_MEMBER
           bool Destroy();
   
         ECAL_API_EXPORTED_MEMBER
           bool SetID(const std::set<long long>& filter_ids_);
   
         ECAL_API_EXPORTED_MEMBER
           bool SetAttribute(const std::string& attr_name_, const std::string& attr_value_);
   
         ECAL_API_EXPORTED_MEMBER
           bool ClearAttribute(const std::string& attr_name_);
   
         ECAL_API_EXPORTED_MEMBER
           bool ReceiveBuffer(std::string& buf_, long long* time_ = nullptr, int rcv_timeout_ = 0) const;
   
         ECAL_API_EXPORTED_MEMBER
           bool AddReceiveCallback(ReceiveCallbackT callback_);
   
         ECAL_API_EXPORTED_MEMBER
           bool AddReceiveCallback(v6::ReceiveCallbackT callback_);
   
         ECAL_API_EXPORTED_MEMBER
           bool RemReceiveCallback();
   
         ECAL_API_EXPORTED_MEMBER
           bool AddEventCallback(eCAL_Subscriber_Event type_, SubEventCallbackT callback_);
   
         ECAL_API_EXPORTED_MEMBER
           bool RemEventCallback(eCAL_Subscriber_Event type_);
   
         ECAL_API_EXPORTED_MEMBER
           bool IsCreated() const { return(m_subscriber_impl != nullptr); }
   
         ECAL_API_EXPORTED_MEMBER
           bool IsPublished() const;
   
         ECAL_API_EXPORTED_MEMBER
           size_t GetPublisherCount() const;
   
         ECAL_API_EXPORTED_MEMBER
           std::string GetTopicName() const;
   
         ECAL_API_EXPORTED_MEMBER
           Registration::STopicId GetId() const;
   
         ECAL_API_EXPORTED_MEMBER
           SDataTypeInformation GetDataTypeInformation() const;
   
         ECAL_API_EXPORTED_MEMBER
           std::string Dump(const std::string& indent_ = "") const;
   
       private:
         // class members
         std::shared_ptr<CSubscriberImpl> m_subscriber_impl;
       };
     }
   }
