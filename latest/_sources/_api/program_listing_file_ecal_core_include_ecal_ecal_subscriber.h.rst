
.. _program_listing_file_ecal_core_include_ecal_ecal_subscriber.h:

Program Listing for File ecal_subscriber.h
==========================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_subscriber.h>` (``ecal/core/include/ecal/ecal_subscriber.h``)

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
   #include <ecal/ecal_types.h>
   
   #include <memory>
   #include <string>
   
   namespace eCAL
   {
     class CSubscriberImpl;
   
     inline namespace v6
     {
       class ECAL_API_CLASS CSubscriber
       {
       public:
         ECAL_API_EXPORTED_MEMBER
           CSubscriber(const std::string& topic_name_, const SDataTypeInformation& data_type_info_ = SDataTypeInformation(), const Subscriber::Configuration& config_ = GetSubscriberConfiguration());
   
         ECAL_API_EXPORTED_MEMBER
           CSubscriber(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const SubEventCallbackT event_callback_, const Subscriber::Configuration& config_ = GetSubscriberConfiguration());
   
         ECAL_API_EXPORTED_MEMBER
           virtual ~CSubscriber();
   
         CSubscriber(const CSubscriber&) = delete;
   
         CSubscriber& operator=(const CSubscriber&) = delete;
   
         ECAL_API_EXPORTED_MEMBER
           CSubscriber(CSubscriber&& rhs) noexcept;
   
         ECAL_API_EXPORTED_MEMBER
           CSubscriber& operator=(CSubscriber&& rhs) noexcept;
   
         ECAL_API_EXPORTED_MEMBER
           bool SetReceiveCallback(ReceiveCallbackT callback_);
   
         ECAL_API_EXPORTED_MEMBER
           bool RemoveReceiveCallback();
   
         ECAL_API_EXPORTED_MEMBER
           size_t GetPublisherCount() const;
   
         ECAL_API_EXPORTED_MEMBER
           std::string GetTopicName() const;
   
         ECAL_API_EXPORTED_MEMBER
           Registration::STopicId GetTopicId() const;
   
         ECAL_API_EXPORTED_MEMBER
           SDataTypeInformation GetDataTypeInformation() const;
   
       private:
         std::shared_ptr<CSubscriberImpl> m_subscriber_impl;
       };
     }
   }
