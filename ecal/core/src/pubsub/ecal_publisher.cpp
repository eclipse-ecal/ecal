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
 * @brief  common data publisher based on eCAL
**/

#include <ecal/ecal.h>

#include "ecal_globals.h"
#include "ecal_publisher_impl.h"
#include "readwrite/ecal_writer_buffer_payload.h"

#include "config/builder/writer_attribute_builder.h"
#include "ecal/ecal_config.h"

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

    // create datawriter
    m_datawriter = std::make_shared<CDataWriter>(data_type_info_, BuildWriterAttributes(topic_name_, config_, GetTransportLayerConfiguration(), GetRegistrationConfiguration()));

    // register datawriter
    g_pubgate()->Register(topic_name_, m_datawriter);
  }

  CPublisher::~CPublisher()
  {
    if (m_datawriter == nullptr) return;

    // unregister datawriter
    if (g_pubgate() != nullptr) g_pubgate()->Unregister(m_datawriter->GetTopicName(), m_datawriter);
#ifndef NDEBUG
    // log it
    eCAL::Logging::Log(log_level_debug1, std::string(m_datawriter->GetTopicName() + "::CPublisher::Destroy"));
#endif
  }

  /**
   * @brief CPublisher are move-enabled
  **/
  CPublisher::CPublisher(CPublisher&& rhs) noexcept :
                m_datawriter(std::move(rhs.m_datawriter))
  {
    rhs.m_datawriter = nullptr;
  }

  /**
   * @brief CPublisher are move-enabled
  **/
  CPublisher& CPublisher::operator=(CPublisher&& rhs) noexcept
  {
    m_datawriter = std::move(rhs.m_datawriter);
    rhs.m_datawriter = nullptr;
    return *this;
  }

  size_t CPublisher::Send(const void* const buf_, const size_t len_, const long long time_ /* = DEFAULT_TIME_ARGUMENT */)
  {
    CBufferPayloadWriter payload{ buf_, len_ };
    return Send(payload, time_);
  }
  
  size_t CPublisher::Send(CPayloadWriter& payload_, long long time_)
  {
    if (m_datawriter == nullptr) return 0;

     // in an optimization case the
     // publisher can send an empty package
     // or we do not have any subscription at all
     // then the data writer will only do some statistics
     // for the monitoring layer and return
     if (!IsSubscribed())
     {
       m_datawriter->RefreshSendCounter();
       return(payload_.GetSize());
     }

     // send content via data writer layer
     const long long write_time = (time_ == DEFAULT_TIME_ARGUMENT) ? eCAL::Time::GetMicroSeconds() : time_;
     const size_t written_bytes = m_datawriter->Write(payload_, write_time, 0);

     // return number of bytes written
     return written_bytes;
  }

  size_t CPublisher::Send(const std::string& s_, long long time_)
  {
    return(Send(s_.data(), s_.size(), time_));
  }

  bool CPublisher::AddEventCallback(eCAL_Publisher_Event type_, const PubEventCallbackT& callback_)
  {
    if (m_datawriter == nullptr) return(false);
    RemEventCallback(type_);
    return(m_datawriter->AddEventCallback(type_, callback_));
  }

  bool CPublisher::RemEventCallback(eCAL_Publisher_Event type_)
  {
    if (m_datawriter == nullptr) return(false);
    return(m_datawriter->RemEventCallback(type_));
  }

  bool CPublisher::IsSubscribed() const
  {
#if ECAL_CORE_REGISTRATION
    if(m_datawriter == nullptr) return(false);
    return(m_datawriter->IsSubscribed());
#else  // ECAL_CORE_REGISTRATION
    return(true);
#endif // ECAL_CORE_REGISTRATION
  }

  size_t CPublisher::GetSubscriberCount() const
  {
    if (m_datawriter == nullptr) return(0);
    return(m_datawriter->GetSubscriberCount());
  }

  std::string CPublisher::GetTopicName() const
  {
    if(m_datawriter == nullptr) return("");
    return(m_datawriter->GetTopicName());
  }

  Registration::STopicId CPublisher::GetPublisherId() const
  {
    if (m_datawriter == nullptr) return{};
    return(m_datawriter->GetId());
  }

  SDataTypeInformation CPublisher::GetDataTypeInformation() const
  {
    if (m_datawriter == nullptr) return(SDataTypeInformation{});
    return(m_datawriter->GetDataTypeInformation());
  }
}
