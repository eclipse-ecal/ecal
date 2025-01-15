
.. _program_listing_file_ecal_core_include_ecal_ecal_publisher_v5.h:

Program Listing for File ecal_publisher_v5.h
============================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_publisher_v5.h>` (``ecal/core/include/ecal/ecal_publisher_v5.h``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

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
   
   #pragma once
   
   #include <ecal/ecal_callback_v5.h>
   #include <ecal/ecal_deprecate.h>
   #include <ecal/ecal_os.h>
   #include <ecal/ecal_payload_writer.h>
   #include <ecal/ecal_config.h>
   #include <ecal/ecal_types.h>
   #include <ecal/config/publisher.h>
   
   #include <chrono>
   #include <cstddef>
   #include <memory>
   #include <string>
   
   namespace eCAL
   {
     class CPublisherImpl;
   
     namespace v5
     {
       class ECAL_API_CLASS CPublisher
       {
       public:
         ECAL_API_EXPORTED_MEMBER
           static constexpr long long DEFAULT_TIME_ARGUMENT = -1;  
         ECAL_API_EXPORTED_MEMBER
           CPublisher();
   
         ECAL_API_EXPORTED_MEMBER
           CPublisher(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const Publisher::Configuration& config_ = GetPublisherConfiguration());
   
         ECAL_API_EXPORTED_MEMBER
           explicit CPublisher(const std::string& topic_name_, const Publisher::Configuration& config_ = GetPublisherConfiguration());
   
         ECAL_API_EXPORTED_MEMBER
           virtual ~CPublisher();
   
         CPublisher(const CPublisher&) = delete;
   
         CPublisher& operator=(const CPublisher&) = delete;
   
         ECAL_API_EXPORTED_MEMBER
           CPublisher(CPublisher&& rhs) noexcept;
   
         ECAL_API_EXPORTED_MEMBER
           CPublisher& operator=(CPublisher&& rhs) noexcept;
   
         ECAL_API_EXPORTED_MEMBER
           bool Create(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const Publisher::Configuration& config_ = GetPublisherConfiguration());
   
         ECAL_API_EXPORTED_MEMBER
           bool Create(const std::string& topic_name_);
   
         ECAL_API_EXPORTED_MEMBER
           bool Destroy();
   
         ECAL_API_EXPORTED_MEMBER
           bool SetDataTypeInformation(const SDataTypeInformation& data_type_info_);
   
         ECAL_API_EXPORTED_MEMBER
           bool SetAttribute(const std::string& attr_name_, const std::string& attr_value_);
   
         ECAL_API_EXPORTED_MEMBER
           bool ClearAttribute(const std::string& attr_name_);
   
         ECAL_API_EXPORTED_MEMBER
           bool SetID(long long filter_id_);
   
         ECAL_API_EXPORTED_MEMBER
           size_t Send(const void* buf_, size_t len_, long long time_ = DEFAULT_TIME_ARGUMENT);
   
         ECAL_API_EXPORTED_MEMBER
           size_t Send(CPayloadWriter& payload_, long long time_ = DEFAULT_TIME_ARGUMENT);
   
         ECAL_API_EXPORTED_MEMBER
           size_t Send(const std::string& s_, long long time_ = DEFAULT_TIME_ARGUMENT);
   
         ECAL_API_EXPORTED_MEMBER
           bool AddEventCallback(eCAL_Publisher_Event type_, PubEventCallbackT callback_);
   
         ECAL_API_EXPORTED_MEMBER
           bool RemEventCallback(eCAL_Publisher_Event type_);
   
         ECAL_API_EXPORTED_MEMBER
           bool IsCreated() const { return(m_publisher_impl != nullptr); }
   
         ECAL_API_EXPORTED_MEMBER
           bool IsSubscribed() const;
   
         ECAL_API_EXPORTED_MEMBER
           size_t GetSubscriberCount() const;
   
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
         std::shared_ptr<CPublisherImpl> m_publisher_impl;
         long long                       m_filter_id;
       };
     }
   }
