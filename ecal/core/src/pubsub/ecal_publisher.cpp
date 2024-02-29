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

namespace
{
  // TODO: remove me with new CDescGate
  bool ApplyTopicDescription(const std::string& topic_name_, const eCAL::SDataTypeInformation& data_type_info_)
  {
    if (eCAL::g_descgate() != nullptr)
    {
      // Calculate the quality of the current info
      eCAL::CDescGate::QualityFlags quality = eCAL::CDescGate::QualityFlags::NO_QUALITY;
      if (!data_type_info_.name.empty() || !data_type_info_.encoding.empty())
        quality |= eCAL::CDescGate::QualityFlags::TYPE_AVAILABLE;
      if (!data_type_info_.descriptor.empty())
        quality |= eCAL::CDescGate::QualityFlags::DESCRIPTION_AVAILABLE;
      quality |= eCAL::CDescGate::QualityFlags::INFO_COMES_FROM_THIS_PROCESS;
      quality |= eCAL::CDescGate::QualityFlags::INFO_COMES_FROM_CORRECT_ENTITY;
      quality |= eCAL::CDescGate::QualityFlags::INFO_COMES_FROM_PRODUCER;

      return eCAL::g_descgate()->ApplyTopicDescription(topic_name_, data_type_info_, quality);
    }
    return false;
  }
}

namespace eCAL
{
  CPublisher::CPublisher() :
    m_datawriter(nullptr),
    m_id(0),
    m_created(false),
    m_initialized(false)
  {
  }

  CPublisher::CPublisher(const std::string& topic_name_, const SDataTypeInformation& data_type_info_)
    : CPublisher()
  {
    CPublisher::Create(topic_name_, data_type_info_);
  }

  CPublisher::CPublisher(const std::string& topic_name_) 
    : CPublisher(topic_name_, SDataTypeInformation{})
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
                m_created(rhs.m_created),
                m_initialized(rhs.m_initialized)
  {
    rhs.m_created     = false;
    rhs.m_initialized = false;
  }

  /**
   * @brief CPublisher are move-enabled
  **/
  CPublisher& CPublisher::operator=(CPublisher&& rhs) noexcept
  {
    // Call destroy, to clean up the current state, then afterwards move all elements
    Destroy();

    m_datawriter      = std::move(rhs.m_datawriter);
    m_id              = rhs.m_id;
    m_created         = rhs.m_created;
    m_initialized     = rhs.m_initialized;

    rhs.m_created     = false;
    rhs.m_initialized = false;

    return *this;
  }

  bool CPublisher::Create(const std::string& topic_name_, const SDataTypeInformation& data_type_info_)
  {
    if (m_created)              return(false);
    if (topic_name_.empty())    return(false);
    if (g_globals() == nullptr) return(false);

    // initialize globals
    if (g_globals()->IsInitialized(Init::Publisher) == 0)
    {
      g_globals()->Initialize(Init::Publisher);
      m_initialized = true;
    }

    // create data writer
    m_datawriter = std::make_shared<CDataWriter>();
    if (!m_datawriter->Create(topic_name_, data_type_info_))
    {
#ifndef NDEBUG
      // log it
      if (g_log() != nullptr) g_log()->Log(log_level_debug1, topic_name_ + "::CPublisher::Create - FAILED");
#endif
      return(false);
    }
#ifndef NDEBUG
    // log it
    if (g_log() != nullptr) g_log()->Log(log_level_debug1, topic_name_ + "::CPublisher::Create - SUCCESS");
#endif
    // register publisher gateway (for publisher memory file and event name)
    g_pubgate()->Register(topic_name_, m_datawriter);

    // register to description gateway for type / description checking
    ApplyTopicDescription(topic_name_, data_type_info_);

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
    if(!m_created)             return(false);
    if(g_globals() == nullptr) return(false);

    // destroy data writer
    m_datawriter->Destroy();

    // unregister data writer
    if(g_pubgate() != nullptr) g_pubgate()->Unregister(m_datawriter->GetTopicName(), m_datawriter);
#ifndef NDEBUG
    // log it
    if (g_log() != nullptr) g_log()->Log(log_level_debug1, std::string(m_datawriter->GetTopicName() + "::CPublisher::Destroy"));
#endif

    // free datawriter
    m_datawriter.reset();

    // we made it :-)
    m_created = false;

    // if we initialize the globals then we finalize 
    // here to decrease reference counter
    if (m_initialized)
    {
      g_globals()->Finalize(Init::Publisher);
      m_initialized = false;
    }

    return(true);
  }

  bool CPublisher::SetDataTypeInformation(const SDataTypeInformation& data_type_info_)
  {
    if (m_datawriter == nullptr) return false;
    ApplyTopicDescription(m_datawriter->GetTopicName(), data_type_info_);
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

  bool CPublisher::ShareType(bool state_ /*= true*/)
  {
    if (m_datawriter == nullptr) return false;
    m_datawriter->ShareType(state_);
    return true;
  }

  bool CPublisher::ShareDescription(bool state_ /*= true*/)
  {
    if (m_datawriter == nullptr) return false;
    m_datawriter->ShareDescription(state_);
    return true;
  }

  bool CPublisher::SetID(long long id_)
  {
    m_id = id_;
    return(true);
  }

  size_t CPublisher::Send(const void* const buf_, const size_t len_, const long long time_ /* = DEFAULT_TIME_ARGUMENT */) const
  {
    CBufferPayloadWriter payload{ buf_, len_ };
    return Send(payload, time_);
  }
  
  size_t CPublisher::Send(CPayloadWriter& payload_, long long time_) const
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

  size_t CPublisher::Send(const std::string& s_, long long time_) const
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
