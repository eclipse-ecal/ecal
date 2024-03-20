
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
   #include <ecal/ecal_qos.h>
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
   
       ECAL_DEPRECATE_SINCE_5_13("Please use the constructor CSubscriber(const std::string& topic_name_, const SDataTypeInformation& topic_info_) instead. This function will be removed in future eCAL versions.")
       ECAL_API CSubscriber(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_ = "");
   
       ECAL_API CSubscriber(const std::string& topic_name_, const SDataTypeInformation& topic_info_);
   
       ECAL_API CSubscriber(const std::string& topic_name_);
   
       ECAL_API virtual ~CSubscriber();
   
       ECAL_API CSubscriber(const CSubscriber&) = delete;
   
       ECAL_API CSubscriber& operator=(const CSubscriber&) = delete;
   
       ECAL_API CSubscriber(CSubscriber&& rhs) noexcept;
   
       ECAL_API CSubscriber& operator=(CSubscriber&& rhs) noexcept;
   
       ECAL_DEPRECATE_SINCE_5_13("Please use the create method bool Create(const std::string& topic_name_, const STopicInformation& topic_info_) instead. This function will be removed in future eCAL versions.")
       ECAL_API bool Create(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_ = "");
   
       ECAL_API bool Create(const std::string& topic_name_) {
         return Create(topic_name_, SDataTypeInformation{});
       }
   
       ECAL_API bool Create(const std::string& topic_name_, const SDataTypeInformation& topic_info_);
   
       ECAL_API bool Destroy();
   
       ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
       ECAL_API bool SetQOS(const QOS::SReaderQOS& qos_);
   
       ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
       ECAL_API QOS::SReaderQOS GetQOS();
   
       ECAL_API bool SetID(const std::set<long long>& id_set_);
   
       ECAL_API bool SetAttribute(const std::string& attr_name_, const std::string& attr_value_);
   
       ECAL_API bool ClearAttribute(const std::string& attr_name_);
   
       ECAL_DEPRECATE_SINCE_5_10("Please use the method bool ReceiveBuffer(std::string& buf_, long long* time_ = nullptr, int rcv_timeout_ = 0) instead. This function will be removed in future eCAL versions.")
       ECAL_API size_t Receive(std::string& buf_, long long* time_ = nullptr, int rcv_timeout_ = 0) const;
   
       ECAL_API bool ReceiveBuffer(std::string& buf_, long long* time_ = nullptr, int rcv_timeout_ = 0) const;
   
       ECAL_API bool AddReceiveCallback(ReceiveCallbackT callback_);
   
       ECAL_API bool RemReceiveCallback();
   
       ECAL_API bool AddEventCallback(eCAL_Subscriber_Event type_, SubEventCallbackT callback_);
   
       ECAL_API bool RemEventCallback(eCAL_Subscriber_Event type_);
   
       ECAL_API bool IsCreated() const {return(m_created);}
   
       ECAL_API size_t GetPublisherCount() const;
   
       ECAL_API std::string GetTopicName() const;
   
       ECAL_DEPRECATE_SINCE_5_13("Please use the method SDataTypeInformation GetDataTypeInformation() instead. You can extract the typename from the SDataTypeInformation variable. This function will be removed in future eCAL versions.")
       ECAL_API std::string GetTypeName() const;
   
       ECAL_DEPRECATE_SINCE_5_13("Please use the method SDataTypeInformation GetDataTypeInformation() instead. You can extract the descriptor from the SDataTypeInformation variable. This function will be removed in future eCAL versions.")
       ECAL_API std::string GetDescription() const;
   
       ECAL_API SDataTypeInformation GetDataTypeInformation() const;
   
       ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
       ECAL_API bool SetTimeout(int timeout_);
   
       ECAL_API std::string Dump(const std::string& indent_ = "") const;
   
     protected:
       void InitializeQOS();
       bool ApplyTopicToDescGate(const std::string& topic_name_, const SDataTypeInformation& topic_info_);
   
       // class members
       std::shared_ptr<CDataReader>     m_datareader;
       struct ECAL_API QOS::SReaderQOS  m_qos;
       bool                             m_created;
       bool                             m_initialized;
     };
   }
