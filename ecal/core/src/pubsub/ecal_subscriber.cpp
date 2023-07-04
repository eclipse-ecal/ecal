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
 * @brief  common data subscriber for eCAL
**/

#include <ecal/ecal.h>

#include "ecal_globals.h"
#include "readwrite/ecal_reader.h"

#include <sstream>
#include <iostream>


namespace eCAL
{
  CSubscriber::CSubscriber() :
                 m_datareader(nullptr),
                 m_created(false),
                 m_initialized(false)
  {
    InitializeQOS();
  }

  CSubscriber::CSubscriber(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_ /* = "" */)
    : CSubscriber()
  {
    Create(topic_name_, topic_type_, topic_desc_);
  }

  CSubscriber::CSubscriber(const std::string& topic_name_, const STopicInformation& topic_info_)
    : CSubscriber()
  {
    Create(topic_name_, topic_info_);
  }

  CSubscriber::CSubscriber(const std::string& topic_name_)
    : CSubscriber(topic_name_, STopicInformation{})
  {}

  CSubscriber::~CSubscriber()
  {
    Destroy();
  }

  CSubscriber::CSubscriber(CSubscriber&& rhs) noexcept :
                 m_datareader(rhs.m_datareader),
                 m_qos(rhs.m_qos),
                 m_created(rhs.m_created),
                 m_initialized(rhs.m_initialized)
  {
    InitializeQOS();

    rhs.m_created     = false;
    rhs.m_initialized = false;
  }

  CSubscriber& CSubscriber::operator=(CSubscriber&& rhs) noexcept
  {
    m_datareader      = std::move(rhs.m_datareader);

    m_qos             = rhs.m_qos;
    m_created         = rhs.m_created;
    m_initialized     = rhs.m_initialized;

    InitializeQOS();
    rhs.m_created     = false;
    rhs.m_initialized = false;

    return *this;
  };

  bool CSubscriber::Create(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_ /* = "" */)
  {
    STopicInformation info;
    auto split_type = Util::SplitCombinedTopicType(topic_type_);
    info.encoding = split_type.first;
    info.type = split_type.second;
    info.descriptor = topic_desc_;
    return Create(topic_name_, info);
  }

  bool CSubscriber::Create(const std::string& topic_name_, const STopicInformation& topic_info_)
  {
    if (m_created)              return(false);
    if (g_globals() == nullptr) return(false);
    if (topic_name_.empty())    return(false);

    // initialize globals
    if (g_globals()->IsInitialized(Init::Subscriber) == 0)
    {
      g_globals()->Initialize(Init::Subscriber);
      m_initialized = true;
    }

    // create data reader
    m_datareader = std::make_shared<CDataReader>();
    // set qos
    m_datareader->SetQOS(m_qos);
    // create it
    if (!m_datareader->Create(topic_name_, topic_info_))
    {
#ifndef NDEBUG
      // log it
      if (g_log() != nullptr) g_log()->Log(log_level_debug1, std::string(topic_name_ + "::CSubscriber::Create - FAILED"));
#endif
      return(false);
    }
#ifndef NDEBUG
    // log it
    if (g_log() != nullptr) g_log()->Log(log_level_debug1, std::string(topic_name_ + "::CSubscriber::Create - SUCCESS"));
#endif
    // register to subscriber gateway for publisher memory file receive thread
    g_subgate()->Register(topic_name_, m_datareader);

    // register to description gateway for type / description checking
    ApplyTopicToDescGate(topic_name_, topic_info_);

    // we made it :-)
    m_created = true;

    return(m_created);
  }

  bool CSubscriber::Destroy()
  {
    if(!m_created)             return(false);
    if(g_globals() == nullptr) return(false);

    // remove receive callback
    RemReceiveCallback();

    // first unregister data reader
    if(g_subgate() != nullptr) g_subgate()->Unregister(m_datareader->GetTopicName(), m_datareader);
#ifndef NDEBUG
    // log it
    if (g_log() != nullptr) g_log()->Log(log_level_debug1, std::string(m_datareader->GetTopicName() + "::CSubscriber::Destroy"));
#endif

    // destroy local data reader
    m_datareader->Destroy();

    // free datareader
    m_datareader.reset();
    
    // we made it :-)
    m_created = false;

    // if we initialize the globals then we finalize 
    // here to decrease reference counter
    if (m_initialized)
    {
      g_globals()->Finalize(Init::Subscriber);
      m_initialized = false;
    }

    return(true);
  }

  bool CSubscriber::SetQOS(const QOS::SReaderQOS& qos_)
  {
    if (m_created) return false;
    m_qos = qos_;
    return true;
  }

  QOS::SReaderQOS CSubscriber::GetQOS()
  {
    return m_qos;
  }

  bool CSubscriber::SetID(const std::set<long long>& id_set_)
  {
    if (m_datareader == nullptr) return(false);
    m_datareader->SetID(id_set_);
    return(true);
  }

  bool CSubscriber::SetAttribute(const std::string& attr_name_, const std::string& attr_value_)
  {
    if(m_datareader == nullptr) return false;
    return m_datareader->SetAttribute(attr_name_, attr_value_);
  }

  bool CSubscriber::ClearAttribute(const std::string& attr_name_)
  {
    if(m_datareader == nullptr) return false;
    return m_datareader->ClearAttribute(attr_name_);
  }

  size_t CSubscriber::Receive(std::string& buf_, long long* time_ /* = nullptr */, int rcv_timeout_ /* = 0 */) const
  {
    if(!m_created) return(0);
    if (m_datareader->Receive(buf_, time_, rcv_timeout_))
    {
      return(buf_.size());
    }
    return(0);
  }

  bool CSubscriber::ReceiveBuffer(std::string& buf_, long long* time_ /* = nullptr */, int rcv_timeout_ /* = 0 */) const
  {
    if (!m_created) return(false);
    return(m_datareader->Receive(buf_, time_, rcv_timeout_));
  }

  bool CSubscriber::AddReceiveCallback(ReceiveCallbackT callback_)
  {
    if(m_datareader == nullptr) return(false);
    RemReceiveCallback();
    return(m_datareader->AddReceiveCallback(callback_));
  }

  bool CSubscriber::RemReceiveCallback()
  {
    if(m_datareader == nullptr) return(false);
    return(m_datareader->RemReceiveCallback());
  }

  bool CSubscriber::AddEventCallback(eCAL_Subscriber_Event type_, SubEventCallbackT callback_)
  {
    if (m_datareader == nullptr) return(false);
    RemEventCallback(type_);
    return(m_datareader->AddEventCallback(type_, callback_));
  }

  bool CSubscriber::RemEventCallback(eCAL_Subscriber_Event type_)
  {
    if (m_datareader == nullptr) return(false);
    return(m_datareader->RemEventCallback(type_));
  }

  size_t CSubscriber::GetPublisherCount() const
  {
    if(m_datareader == nullptr) return(0);
    return(m_datareader->GetPublisherCount());
  }

  std::string CSubscriber::GetTopicName() const
  {
    if(m_datareader == nullptr) return("");
    return(m_datareader->GetTopicName());
  }

  std::string CSubscriber::GetTypeName() const
  {
    if(m_datareader == nullptr) return("");
    STopicInformation info = m_datareader->GetTopicInformation();
    return(Util::CombinedTopicEncodingAndType(info.encoding, info.type));
  }

  std::string CSubscriber::GetDescription() const
  {
    if(m_datareader == nullptr) return("");
    return(m_datareader->GetTopicInformation().descriptor);
  }
  
  STopicInformation CSubscriber::GetTopicInformation() const
  {
    if (m_datareader == nullptr) return(STopicInformation{});
    return(m_datareader->GetTopicInformation());
  }

  bool CSubscriber::SetTimeout(int timeout_)
  {
    if (m_datareader == nullptr) return(false);
    return(m_datareader->SetTimeout(timeout_));
  }

  void CSubscriber::InitializeQOS()
  {
    m_qos = QOS::SReaderQOS();
    m_qos.reliability = QOS::best_effort_reliability_qos;
  }

  bool CSubscriber::ApplyTopicToDescGate(const std::string& topic_name_, const STopicInformation& topic_info_)
  {
    if (g_descgate() != nullptr)
    {
      // Calculate the quality of the current info
      ::eCAL::CDescGate::QualityFlags quality = ::eCAL::CDescGate::QualityFlags::NO_QUALITY;
      if (!topic_info_.type.empty() || !topic_info_.encoding.empty())
        quality |= ::eCAL::CDescGate::QualityFlags::TYPE_AVAILABLE;
      if (!topic_info_.descriptor.empty())
        quality |= ::eCAL::CDescGate::QualityFlags::DESCRIPTION_AVAILABLE;
      quality |= ::eCAL::CDescGate::QualityFlags::INFO_COMES_FROM_THIS_PROCESS;
      quality |= ::eCAL::CDescGate::QualityFlags::INFO_COMES_FROM_CORRECT_ENTITY;

      return g_descgate()->ApplyTopicDescription(topic_name_, topic_info_, quality);
    }
    return false;
  }

  std::string CSubscriber::Dump(const std::string& indent_ /* = "" */) const
  {
    std::stringstream out;

    out << indent_ << "----------------------" << std::endl;
    out << indent_ << " class CSubscriber    " << std::endl;
    out << indent_ << "----------------------" << std::endl;
    out << indent_ << "m_created:            " << m_created << std::endl;
    if((m_datareader != nullptr) && m_datareader->IsCreated()) out << indent_ << m_datareader->Dump("    ");
    out << std::endl;

    return(out.str());
  }
}
