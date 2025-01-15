
.. _program_listing_file_ecal_core_include_ecal_ecal_publisher.h:

Program Listing for File ecal_publisher.h
=========================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_publisher.h>` (``ecal/core/include/ecal/ecal_publisher.h``)

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
   
   #include <ecal/ecal_deprecate.h>
   #include <ecal/ecal_os.h>
   
   #include <ecal/ecal_callback.h>
   #include <ecal/ecal_config.h>
   #include <ecal/ecal_payload_writer.h>
   #include <ecal/ecal_types.h>
   
   #include <memory>
   #include <string>
   
   namespace eCAL
   {
     class CPublisherImpl;
   
     inline namespace v6
     {
       class ECAL_API_CLASS CPublisher
       {
       public:
         ECAL_API_EXPORTED_MEMBER
           static constexpr long long DEFAULT_TIME_ARGUMENT = -1;  
         ECAL_API_EXPORTED_MEMBER
           CPublisher(const std::string& topic_name_, const SDataTypeInformation& data_type_info_ = SDataTypeInformation(), const Publisher::Configuration& config_ = GetPublisherConfiguration());
   
         ECAL_API_EXPORTED_MEMBER
           CPublisher(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const PubEventCallbackT event_callback_, const Publisher::Configuration& config_ = GetPublisherConfiguration());
   
         ECAL_API_EXPORTED_MEMBER
           virtual ~CPublisher();
   
         CPublisher(const CPublisher&) = delete;
   
         CPublisher& operator=(const CPublisher&) = delete;
   
         ECAL_API_EXPORTED_MEMBER
           CPublisher(CPublisher&& rhs) noexcept;
   
         ECAL_API_EXPORTED_MEMBER
           CPublisher& operator=(CPublisher&& rhs) noexcept;
   
         ECAL_API_EXPORTED_MEMBER
           bool Send(const void* buf_, size_t len_, long long time_ = DEFAULT_TIME_ARGUMENT);
   
         ECAL_API_EXPORTED_MEMBER
           bool Send(CPayloadWriter& payload_, long long time_ = DEFAULT_TIME_ARGUMENT);
   
         ECAL_API_EXPORTED_MEMBER
           bool Send(const std::string& payload_, long long time_ = DEFAULT_TIME_ARGUMENT);
   
         ECAL_API_EXPORTED_MEMBER
           size_t GetSubscriberCount() const;
   
         ECAL_API_EXPORTED_MEMBER
           std::string GetTopicName() const;
   
         ECAL_API_EXPORTED_MEMBER
           Registration::STopicId GetTopicId() const;
   
         ECAL_API_EXPORTED_MEMBER
           SDataTypeInformation GetDataTypeInformation() const;
   
       private:
         std::shared_ptr<CPublisherImpl> m_publisher_impl;
       };
     }
   }
