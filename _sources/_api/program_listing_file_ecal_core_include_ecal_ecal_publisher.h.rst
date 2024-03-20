
.. _program_listing_file_ecal_core_include_ecal_ecal_publisher.h:

Program Listing for File ecal_publisher.h
=========================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_publisher.h>` (``ecal/core/include/ecal/ecal_publisher.h``)

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
   #include <ecal/ecal_payload_writer.h>
   #include <ecal/ecal_qos.h>
   #include <ecal/ecal_tlayer.h>
   #include <ecal/ecal_types.h>
   
   #include <chrono>
   #include <memory>
   #include <string>
   
   namespace eCAL
   {
     class CDataWriter;
   
     class CPublisher
     {
     public:
   
       ECAL_API static constexpr long long DEFAULT_TIME_ARGUMENT        = -1;  
       ECAL_API static constexpr long long DEFAULT_ACKNOWLEDGE_ARGUMENT = -1;  
       ECAL_API CPublisher();
   
       ECAL_DEPRECATE_SINCE_5_13("Please use the constructor CPublisher(const std::string& topic_name_, const SDataTypeInformation& topic_info_) instead. This function will be removed in future eCAL versions.")
       ECAL_API CPublisher(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_ = "");
   
       ECAL_API CPublisher(const std::string& topic_name_, const SDataTypeInformation& topic_info_);
   
       ECAL_API CPublisher(const std::string& topic_name_);
   
       ECAL_API virtual ~CPublisher();
   
       ECAL_API CPublisher(const CPublisher&) = delete;
   
       ECAL_API CPublisher& operator=(const CPublisher&) = delete;
   
       ECAL_API CPublisher(CPublisher&& rhs) noexcept;
   
       ECAL_API CPublisher& operator=(CPublisher&& rhs) noexcept;
   
       ECAL_DEPRECATE_SINCE_5_13("Please use the create method bool Create(const std::string& topic_name_, const SDataTypeInformation& topic_info_) instead. This function will be removed in future eCAL versions.")
       ECAL_API bool Create(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_ = "");
   
       ECAL_API bool Create(const std::string& topic_name_, const SDataTypeInformation& topic_info_);
   
       ECAL_API bool Create(const std::string& topic_name_)
       {
         return Create(topic_name_, SDataTypeInformation());
       }
   
       ECAL_API bool Destroy();
   
       ECAL_DEPRECATE_SINCE_5_13("Please use the method bool SetDataTypeInformation(const SDataTypeInformation& topic_info_) instead. This function will be removed in future eCAL versions.")
       ECAL_API bool SetTypeName(const std::string& topic_type_name_);
   
       ECAL_DEPRECATE_SINCE_5_13("Please use the method bool SetDataTypeInformation(const SDataTypeInformation& topic_info_) instead. This function will be removed in future eCAL versions.")
       ECAL_API bool SetDescription(const std::string& topic_desc_);
   
       ECAL_API bool SetDataTypeInformation(const SDataTypeInformation& topic_info_);
   
       ECAL_API bool SetAttribute(const std::string& attr_name_, const std::string& attr_value_);
   
       ECAL_API bool ClearAttribute(const std::string& attr_name_);
   
       ECAL_API bool ShareType(bool state_ = true);
   
       ECAL_API bool ShareDescription(bool state_ = true);
   
       ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
       ECAL_API bool SetQOS(const QOS::SWriterQOS& qos_);
   
       ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
       ECAL_API QOS::SWriterQOS GetQOS();
   
       ECAL_API bool SetLayerMode(TLayer::eTransportLayer layer_, TLayer::eSendMode mode_);
   
       ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
       ECAL_API bool SetMaxBandwidthUDP(long bandwidth_);
   
       ECAL_API bool ShmSetBufferCount(long buffering_);
   
       ECAL_API bool ShmEnableZeroCopy(bool state_);
   
       ECAL_API bool ShmSetAcknowledgeTimeout(long long acknowledge_timeout_ms_);
   
       template <typename Rep, typename Period>
       bool ShmSetAcknowledgeTimeout(std::chrono::duration<Rep, Period> acknowledge_timeout_)
       {
         auto acknowledge_timeout_ms = std::chrono::duration_cast<std::chrono::milliseconds>(acknowledge_timeout_).count();
         return ShmSetAcknowledgeTimeout(static_cast<long long>(acknowledge_timeout_ms));
       }
   
       ECAL_API bool SetID(long long id_);
   
       ECAL_API size_t Send(const void* buf_, size_t len_, long long time_ = DEFAULT_TIME_ARGUMENT) const;
   
       ECAL_API size_t Send(CPayloadWriter& payload_, long long time_ = DEFAULT_TIME_ARGUMENT) const;
   
       ECAL_API size_t Send(const void* buf_, size_t len_, long long time_, long long acknowledge_timeout_ms_) const;
   
       ECAL_DEPRECATE_SINCE_5_12("Please use the method size_t Send(CPayloadWriter& payload_, long long time_, long long acknowledge_timeout_ms_) const instead. This function will be removed in future eCAL versions.")
       ECAL_API size_t SendSynchronized(const void* const buf_, size_t len_, long long time_, long long acknowledge_timeout_ms_) const
       {
         return Send(buf_, len_, time_, acknowledge_timeout_ms_);
       }
   
       ECAL_API size_t Send(CPayloadWriter& payload_, long long time_, long long acknowledge_timeout_ms_) const;
   
       ECAL_API size_t Send(const std::string& s_, long long time_ = DEFAULT_TIME_ARGUMENT) const
       {
         return(Send(s_.data(), s_.size(), time_, DEFAULT_ACKNOWLEDGE_ARGUMENT));
       }
   
       ECAL_API size_t Send(const std::string& s_, long long time_, long long acknowledge_timeout_ms_) const
       {
         return(Send(s_.data(), s_.size(), time_, acknowledge_timeout_ms_));
       }
   
       ECAL_API bool AddEventCallback(eCAL_Publisher_Event type_, PubEventCallbackT callback_);
   
       ECAL_API bool RemEventCallback(eCAL_Publisher_Event type_);
   
       ECAL_API bool IsCreated() const {return(m_created);}
   
       ECAL_API bool IsSubscribed() const;
   
       ECAL_API size_t GetSubscriberCount() const;
   
       ECAL_API std::string GetTopicName() const;
   
       ECAL_DEPRECATE_SINCE_5_13("Please use the method SDataTypeInformation GetDataTypeInformation() instead. You can extract the typename from the SDataTypeInformation variable. This function will be removed in future eCAL versions.")
       ECAL_API std::string GetTypeName() const;
   
       ECAL_DEPRECATE_SINCE_5_13("Please use the method SDataTypeInformation GetDataTypeInformation() instead. You can extract the descriptor from the SDataTypeInformation variable. This function will be removed in future eCAL versions.")
       ECAL_API std::string GetDescription() const;
   
       ECAL_API SDataTypeInformation GetDataTypeInformation() const;
   
       ECAL_API std::string Dump(const std::string& indent_ = "") const;
   
     protected:
       void InitializeQOS();
       void InitializeTLayer();
       bool ApplyTopicToDescGate(const std::string& topic_name_, const SDataTypeInformation& topic_info_);
   
       // class members
       std::shared_ptr<CDataWriter>     m_datawriter;
       struct ECAL_API QOS::SWriterQOS  m_qos;
       struct ECAL_API TLayer::STLayer  m_tlayer;
       long long                        m_id;
       bool                             m_created;
       bool                             m_initialized;
     };
   }
