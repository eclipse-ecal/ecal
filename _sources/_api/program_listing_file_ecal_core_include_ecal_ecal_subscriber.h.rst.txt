
.. _program_listing_file_ecal_core_include_ecal_ecal_subscriber.h:

Program Listing for File ecal_subscriber.h
==========================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_subscriber.h>` (``ecal/core/include/ecal/ecal_subscriber.h``)

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
   
   #include <cstddef>
   #include <ecal/ecal_callback.h>
   #include <ecal/ecal_deprecate.h>
   #include <ecal/ecal_os.h>
   #include <ecal/ecal_types.h>
   
   #include <memory>
   #include <set>
   #include <string>
   
   namespace eCAL
   {
     class CDataReader;
   
     class CSubscriber
     {
     public:
       ECAL_API CSubscriber();
   
       ECAL_API CSubscriber(const std::string& topic_name_, const SDataTypeInformation& topic_info_);
   
       ECAL_API explicit CSubscriber(const std::string& topic_name_);
   
       ECAL_API virtual ~CSubscriber();
   
       CSubscriber(const CSubscriber&) = delete;
   
       CSubscriber& operator=(const CSubscriber&) = delete;
   
       ECAL_API CSubscriber(CSubscriber&& rhs) noexcept;
   
       ECAL_API CSubscriber& operator=(CSubscriber&& rhs) noexcept;
   
       ECAL_API bool Create(const std::string& topic_name_);
   
       ECAL_API bool Create(const std::string& topic_name_, const SDataTypeInformation& topic_info_);
   
       ECAL_API bool Destroy();
   
       ECAL_API bool SetID(const std::set<long long>& id_set_);
   
       ECAL_API bool SetAttribute(const std::string& attr_name_, const std::string& attr_value_);
   
       ECAL_API bool ClearAttribute(const std::string& attr_name_);
   
       ECAL_API bool ReceiveBuffer(std::string& buf_, long long* time_ = nullptr, int rcv_timeout_ = 0) const;
   
       ECAL_API bool AddReceiveCallback(ReceiveCallbackT callback_);
   
       ECAL_API bool RemReceiveCallback();
   
       ECAL_API bool AddEventCallback(eCAL_Subscriber_Event type_, SubEventCallbackT callback_);
   
       ECAL_API bool RemEventCallback(eCAL_Subscriber_Event type_);
   
       ECAL_API bool IsCreated() const {return(m_created);}
   
       ECAL_API size_t GetPublisherCount() const;
   
       ECAL_API std::string GetTopicName() const;
   
       ECAL_API SDataTypeInformation GetDataTypeInformation() const;
   
       ECAL_API std::string Dump(const std::string& indent_ = "") const;
   
     protected:
       // class members
       std::shared_ptr<CDataReader>     m_datareader;
       bool                             m_created;
       bool                             m_initialized;
     };
   }
