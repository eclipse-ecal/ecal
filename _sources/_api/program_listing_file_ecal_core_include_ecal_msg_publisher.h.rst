
.. _program_listing_file_ecal_core_include_ecal_msg_publisher.h:

Program Listing for File publisher.h
====================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_msg_publisher.h>` (``ecal/core/include/ecal/msg/publisher.h``)

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
   #include <ecal/ecal_publisher.h>
   #include <ecal/ecal_util.h>
   
   #include <string>
   #include <vector>
   #include <functional>
   #include <assert.h>
   #include <string.h>
   
   namespace eCAL
   {
     template <typename T>
     class CMsgPublisher : public CPublisher
     {
     public:
       CMsgPublisher() : CPublisher()
       {
       }
   
       ECAL_DEPRECATE_SINCE_5_13("Please use the constructor CMsgPublisher(const std::string& topic_name_, const SDataTypeInformation& topic_info_) instead. This function will be removed in future eCAL versions.")
       CMsgPublisher(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_ = "") : CPublisher(topic_name_, topic_type_, topic_desc_)
       {
       }
   
       CMsgPublisher(const std::string& topic_name_, const SDataTypeInformation& topic_info_) : CPublisher(topic_name_, topic_info_)
       {
       }
   
       CMsgPublisher(const std::string& topic_name_) : CMsgPublisher(topic_name_, GetDataTypeInformation())
       {
       }
   
       CMsgPublisher(const CMsgPublisher&) = delete;
   
       CMsgPublisher& operator=(const CMsgPublisher&) = delete;
   
       CMsgPublisher(CMsgPublisher&&) = default;
   
       CMsgPublisher& operator=(CMsgPublisher&&) = default;
   
       virtual ~CMsgPublisher() = default;
   
   
   
       ECAL_DEPRECATE_SINCE_5_13("Please use the method Create(const std::string& topic_name_, const SDataTypeInformation& topic_info_) instead. This function will be removed in future eCAL versions.")
       bool Create(const std::string& topic_name_, const std::string& topic_type_ = "", const std::string& topic_desc_ = "")
       {
         return(CPublisher::Create(topic_name_, topic_type_, topic_desc_));
       }
   
       bool Create(const std::string& topic_name_, const SDataTypeInformation& topic_info_)
       {
         return(CPublisher::Create(topic_name_, topic_info_));
       }
   
       bool Destroy()
       {
         return(CPublisher::Destroy());
       }
   
       size_t Send(const T& msg_, long long time_ = eCAL::CPublisher::DEFAULT_TIME_ARGUMENT)
       {
         return Send(msg_, time_, eCAL::CPublisher::DEFAULT_ACKNOWLEDGE_ARGUMENT);
       }
   
       size_t Send(const T& msg_, long long time_, long long acknowledge_timeout_ms_)
       {
         // this is an optimization ...
         // if there is no subscription we do not waste time for
         // serialization but we send an empty payload
         // to still do some statistics like message clock
         // counting and frequency calculation for the monitoring layer
         if (!IsSubscribed())
         {
           return(CPublisher::Send(nullptr, 0, time_, acknowledge_timeout_ms_));
         }
   
         // if we have a subscription allocate memory for the
         // binary stream, serialize the message into the
         // buffer and finally send it with a binary publisher
         size_t size = GetSize(msg_);
         if (size > 0)
         {
           m_buffer.resize(size);
           if (Serialize(msg_, &m_buffer[0], m_buffer.size()))
           {
             return(CPublisher::Send(&m_buffer[0], size, time_, acknowledge_timeout_ms_));
           }
         }
         else
         {
           // send a zero payload length message to trigger the subscriber side
           return(CPublisher::Send(nullptr, 0, time_, acknowledge_timeout_ms_));
         }
         return(0);
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
     private:
       virtual size_t GetSize(const T& msg_) const = 0;
       virtual bool Serialize(const T& msg_, char* buffer_, size_t size_) const = 0;
   
       std::vector<char> m_buffer;
     };
   }
