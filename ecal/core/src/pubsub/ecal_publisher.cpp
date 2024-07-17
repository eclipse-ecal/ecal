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
#include "readwrite/ecal_writer.h"
#include "readwrite/ecal_writer_buffer_payload.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

namespace eCAL
{
  CPublisher::CPublisher() :
    m_datawriter(nullptr),
    m_id(0),
    m_created(false)
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
                m_datawriter(std::move(rhs.m_datawriter)),
                m_id(rhs.m_id),
                m_created(rhs.m_created)
  {
    rhs.m_created = false;
  }

  /**
   * @brief CPublisher are move-enabled
  **/
  CPublisher& CPublisher::operator=(CPublisher&& rhs) noexcept
  {
    // Call destroy, to clean up the current state, then afterwards move all elements
    Destroy();

    m_datawriter  = std::move(rhs.m_datawriter);
    m_id          = rhs.m_id;
    m_created     = rhs.m_created;

    rhs.m_created = false;

    return *this;
  }

  bool CPublisher::Create(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const Publisher::Configuration& config_)
  {
    if (m_created)           return(false);
    if (topic_name_.empty()) return(false);

    // create datawriter
    m_datawriter = std::make_shared<CDataWriter>(topic_name_, data_type_info_, config_);

    // register datawriter
    g_pubgate()->Register(topic_name_, m_datawriter);

    // we made it :-)
    m_created = true;
    return(m_created);
  }

  bool CPublisher::Create(const std::string& topic_name_)
  {
    return Create(topic_name_, SDataTypeInformation());
  }

  bool CPublisher::Destroy()
  {
    if(!m_created) return(false);

    // unregister datawriter
    if(g_pubgate() != nullptr) g_pubgate()->Unregister(m_datawriter->GetTopicName(), m_datawriter);
#ifndef NDEBUG
    // log it
    if (g_log() != nullptr) g_log()->Log(log_level_debug1, std::string(m_datawriter->GetTopicName() + "::CPublisher::Destroy"));
#endif

    // stop & destroy datawriter
    m_datawriter->Stop();
    m_datawriter.reset();

    // we made it :-)
    m_created = false;

    return(true);
  }

  bool CPublisher::SetDataTypeInformation(const SDataTypeInformation& data_type_info_)
  {
    if (m_datawriter == nullptr) return false;
    return m_datawriter->SetDataTypeInformation(data_type_info_);
  }

  bool CPublisher::SetAttribute(const std::string& attr_name_, const std::string& attr_value_)
  {
    if(m_datawriter == nullptr) return false;
    return m_datawriter->SetAttribute(attr_name_, attr_value_);
  }

  bool CPublisher::ClearAttribute(const std::string& attr_name_)
  {
    if(m_datawriter == nullptr) return false;
    return m_datawriter->ClearAttribute(attr_name_);
  }

  bool CPublisher::SetID(long long id_)
  {
    m_id = id_;
    return(true);
  }

  size_t CPublisher::Send(const void* const buf_, const size_t len_, const long long time_ /* = DEFAULT_TIME_ARGUMENT */)
  {
    CBufferPayloadWriter payload{ buf_, len_ };
    return Send(payload, time_);
  }
  
  size_t CPublisher::Send(CPayloadWriter& payload_, long long time_)
  {
     if (!m_created) return(0);

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
     const size_t written_bytes = m_datawriter->Write(payload_, write_time, m_id);

     // return number of bytes written
     return written_bytes;
  }

  size_t CPublisher::Send(const std::string& s_, long long time_)
  {
    return(Send(s_.data(), s_.size(), time_));
  }

  bool CPublisher::AddEventCallback(eCAL_Publisher_Event type_, PubEventCallbackT callback_)
  {
    if (m_datawriter == nullptr) return(false);
    RemEventCallback(type_);
    return(m_datawriter->AddEventCallback(type_, std::move(callback_)));
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

  SDataTypeInformation CPublisher::GetDataTypeInformation() const
  {
    if (m_datawriter == nullptr) return(SDataTypeInformation{});
    return(m_datawriter->GetDataTypeInformation());
  }

  std::string CPublisher::Dump(const std::string& indent_ /* = "" */) const
  {
    std::stringstream out;

    out << indent_ << "----------------------" << '\n';
    out << indent_ << " class CPublisher"      << '\n';
    out << indent_ << "----------------------" << '\n';
    out << indent_ << "m_created:            " << m_created << '\n';
    if((m_datawriter != nullptr) && m_datawriter->IsCreated()) out << indent_ << m_datawriter->Dump("    ");
    out << '\n';

    return(out.str());
  }
}