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

/**
 * @brief  eCAL publisher interface (deprecated eCAL5 version)
**/

#include <ecal/ecal.h>
#include <ecal/v5/ecal_publisher.h>

#include "ecal_globals.h"
#include <pubsub/ecal_publisher_impl.h>
#include "ecal_config_internal.h"

#include <pubsub/config/builder/writer_attribute_builder.h>
#include "readwrite/ecal_writer_buffer_payload.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

namespace eCAL
{
  ECAL_CORE_NAMESPACE_V5
  {
    CPublisher::CPublisher() :
      m_publisher_impl(nullptr),
      m_filter_id(0)
    {
    }

    CPublisher::CPublisher(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const Publisher::Configuration& config_)
      : CPublisher()
    {
      CPublisher::Create(topic_name_, data_type_info_, config_);
    }

    CPublisher::CPublisher(const std::string& topic_name_, const Publisher::Configuration& config_)
      : CPublisher(topic_name_, SDataTypeInformation{}, config_)
    {}

    CPublisher::~CPublisher()
    {
      CPublisher::Destroy();
    }

    /**
     * @brief CPublisher are move-enabled
    **/
    CPublisher::CPublisher(CPublisher&& rhs) noexcept :
                  m_publisher_impl(std::move(rhs.m_publisher_impl)),
                  m_filter_id(rhs.m_filter_id)
    {
      rhs.m_publisher_impl = nullptr;
    }

    /**
     * @brief CPublisher are move-enabled
    **/
    CPublisher& CPublisher::operator=(CPublisher&& rhs) noexcept
    {
      // Call destroy, to clean up the current state, then afterwards move all elements
      Destroy();

      m_publisher_impl = std::move(rhs.m_publisher_impl);
      m_filter_id  = rhs.m_filter_id;
      
      rhs.m_publisher_impl = nullptr;

      return *this;
    }

    bool CPublisher::Create(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const Publisher::Configuration& config_)
    {
      if (m_publisher_impl != nullptr) return(false);
      if (topic_name_.empty())     return(false);

      // create publisher
      m_publisher_impl = std::make_shared<CPublisherImpl>(data_type_info_, BuildWriterAttributes(topic_name_, config_, eCAL::GetTransportLayerConfiguration(), eCAL::GetRegistrationConfiguration()));

      // register publisher
      g_pubgate()->Register(topic_name_, m_publisher_impl);

      // we made it :-)
      return(true);
    }

    bool CPublisher::Create(const std::string& topic_name_)
    {
      return Create(topic_name_, SDataTypeInformation());
    }

    bool CPublisher::Destroy()
    {
      if (m_publisher_impl == nullptr) return(false);

      // unregister publisher
      if(g_pubgate() != nullptr) g_pubgate()->Unregister(m_publisher_impl->GetTopicName(), m_publisher_impl);
  #ifndef NDEBUG
      // log it
      eCAL::Logging::Log(Logging::log_level_debug1, std::string(m_publisher_impl->GetTopicName() + "::CPublisher::Destroy"));
  #endif

      // destroy publisher
      m_publisher_impl.reset();

      // we made it :-)
      return(true);
    }

    bool CPublisher::SetDataTypeInformation(const SDataTypeInformation& data_type_info_)
    {
      if (m_publisher_impl == nullptr) return false;
      return m_publisher_impl->SetDataTypeInformation(data_type_info_);
    }

    bool CPublisher::SetAttribute(const std::string& attr_name_, const std::string& attr_value_)
    {
      if(m_publisher_impl == nullptr) return false;
      return m_publisher_impl->SetAttribute(attr_name_, attr_value_);
    }

    bool CPublisher::ClearAttribute(const std::string& attr_name_)
    {
      if(m_publisher_impl == nullptr) return false;
      return m_publisher_impl->ClearAttribute(attr_name_);
    }

    bool CPublisher::SetID(long long filter_id_)
    {
      m_filter_id = filter_id_;
      return true;
    }

    size_t CPublisher::Send(const void* const buf_, const size_t len_, const long long time_ /* = DEFAULT_TIME_ARGUMENT */)
    {
      CBufferPayloadWriter payload{ buf_, len_ };
      return Send(payload, time_);
    }
    
    size_t CPublisher::Send(CPayloadWriter& payload_, long long time_)
    {
      if (m_publisher_impl == nullptr) return 0;

       // in an optimization case the
       // publisher can send an empty package
       // or we do not have any subscription at all
       // then the data writer will only do some statistics
       // for the monitoring layer and return
       if (!IsSubscribed())
       {
         m_publisher_impl->RefreshSendCounter();
         return(payload_.GetSize());
       }

       // send content via data writer layer
       const long long write_time = (time_ == DEFAULT_TIME_ARGUMENT) ? eCAL::Time::GetMicroSeconds() : time_;
       const size_t written_bytes = m_publisher_impl->Write(payload_, write_time, m_filter_id);

       // return number of bytes written
       return written_bytes;
    }

    size_t CPublisher::Send(const std::string& s_, long long time_)
    {
      return(Send(s_.data(), s_.size(), time_));
    }

    bool CPublisher::AddEventCallback(ePublisherEvent type_, PubEventCallbackT callback_)
    {
      if (m_publisher_impl == nullptr) return(false);
      RemEventCallback(type_);
      return(m_publisher_impl->SetEventCallback(type_, std::move(callback_)));
    }

    bool CPublisher::RemEventCallback(ePublisherEvent type_)
    {
      if (m_publisher_impl == nullptr) return(false);
      return(m_publisher_impl->RemoveEventCallback(type_));
    }

    bool CPublisher::IsSubscribed() const
    {
  #if ECAL_CORE_REGISTRATION
      if(m_publisher_impl == nullptr) return(false);
      return(m_publisher_impl->IsSubscribed());
  #else  // ECAL_CORE_REGISTRATION
      return(true);
  #endif // ECAL_CORE_REGISTRATION
    }

    size_t CPublisher::GetSubscriberCount() const
    {
      if (m_publisher_impl == nullptr) return(0);
      return(m_publisher_impl->GetSubscriberCount());
    }

    std::string CPublisher::GetTopicName() const
    {
      if(m_publisher_impl == nullptr) return("");
      return(m_publisher_impl->GetTopicName());
    }

    STopicId CPublisher::GetId() const
    {
      if (m_publisher_impl == nullptr) return{};
      return(m_publisher_impl->GetTopicId());
    }

    SDataTypeInformation CPublisher::GetDataTypeInformation() const
    {
      if (m_publisher_impl == nullptr) return(SDataTypeInformation{});
      return(m_publisher_impl->GetDataTypeInformation());
    }

    std::string CPublisher::Dump(const std::string& indent_ /* = "" */) const
    {
      std::stringstream out;

      out << indent_ << "----------------------" << '\n';
      out << indent_ << " class CPublisher"      << '\n';
      out << indent_ << "----------------------" << '\n';
      out << indent_ << "DUMP NOT SUPPORTED ANYMORE" << '\n';
      out << indent_ << "----------------------" << '\n';
      out << '\n';

      return(out.str());
    }
  }
}
