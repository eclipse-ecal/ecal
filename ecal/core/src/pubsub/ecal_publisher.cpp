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

/**
 * @brief  eCAL publisher interface
**/

#include <ecal/ecal.h>

#include "ecal_globals.h"
#include "ecal_publisher_impl.h"
#include "readwrite/ecal_writer_buffer_payload.h"

#include "config/builder/writer_attribute_builder.h"
#include "ecal_config_internal.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

namespace eCAL
{
  CPublisher::CPublisher(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const Publisher::Configuration& config_)
  {
    if (topic_name_.empty()) return;

    // create publisher
    m_publisher_impl = std::make_shared<CPublisherImpl>(data_type_info_, BuildWriterAttributes(topic_name_, config_, GetTransportLayerConfiguration(), GetRegistrationConfiguration()));

    // register publisher
    g_pubgate()->Register(topic_name_, m_publisher_impl);
  }

  CPublisher::~CPublisher()
  {
    if (m_publisher_impl == nullptr) return;

    // unregister publisher
    if (g_pubgate() != nullptr) g_pubgate()->Unregister(m_publisher_impl->GetTopicName(), m_publisher_impl);
#ifndef NDEBUG
    // log it
    eCAL::Logging::Log(log_level_debug1, std::string(m_publisher_impl->GetTopicName() + "::CPublisher::Destroy"));
#endif
  }

  /**
   * @brief CPublisher are move-enabled
  **/
  CPublisher::CPublisher(CPublisher&& rhs) noexcept :
                m_publisher_impl(std::move(rhs.m_publisher_impl))
  {
    rhs.m_publisher_impl = nullptr;
  }

  /**
   * @brief CPublisher are move-enabled
  **/
  CPublisher& CPublisher::operator=(CPublisher&& rhs) noexcept
  {
    m_publisher_impl = std::move(rhs.m_publisher_impl);
    rhs.m_publisher_impl = nullptr;
    return *this;
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
     const size_t written_bytes = m_publisher_impl->Write(payload_, write_time, 0);

     // return number of bytes written
     return written_bytes;
  }

  size_t CPublisher::Send(const std::string& payload_, long long time_)
  {
    return(Send(payload_.data(), payload_.size(), time_));
  }

  bool CPublisher::AddEventCallback(eCAL_Publisher_Event type_, const PubEventCallbackT& callback_)
  {
    if (m_publisher_impl == nullptr) return(false);
    RemEventCallback(type_);
    return(m_publisher_impl->AddEventCallback(type_, callback_));
  }

  bool CPublisher::RemEventCallback(eCAL_Publisher_Event type_)
  {
    if (m_publisher_impl == nullptr) return(false);
    return(m_publisher_impl->RemEventCallback(type_));
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

  Registration::STopicId CPublisher::GetPublisherId() const
  {
    if (m_publisher_impl == nullptr) return{};
    return(m_publisher_impl->GetId());
  }

  SDataTypeInformation CPublisher::GetDataTypeInformation() const
  {
    if (m_publisher_impl == nullptr) return(SDataTypeInformation{});
    return(m_publisher_impl->GetDataTypeInformation());
  }
}
