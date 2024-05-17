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
 * @brief  common data subscriber for eCAL
**/

#include <ecal/ecal.h>

#include "ecal_globals.h"
#include "readwrite/ecal_reader.h"

#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <utility>

namespace eCAL
{
  CSubscriber::CSubscriber() :
    m_datareader(nullptr),
    m_created(false)
  {
  }

  CSubscriber::CSubscriber(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const Subscriber::Configuration& config_)
    : CSubscriber()
  {
    CSubscriber::Create(topic_name_, data_type_info_, config_);
  }

  CSubscriber::CSubscriber(const std::string& topic_name_, const Subscriber::Configuration& config_)
    : CSubscriber(topic_name_, SDataTypeInformation{}, config_)
  {}

  CSubscriber::~CSubscriber()
  {
    CSubscriber::Destroy();
  }

  CSubscriber::CSubscriber(CSubscriber&& rhs) noexcept :
                 m_datareader(std::move(rhs.m_datareader)),
                 m_created(rhs.m_created)
  {
    rhs.m_created = false;
  }

  CSubscriber& CSubscriber::operator=(CSubscriber&& rhs) noexcept
  {
    // Call destroy, to clean up the current state, then afterwards move all elements
    Destroy();

    m_datareader  = std::move(rhs.m_datareader);
    m_created     = rhs.m_created;

    rhs.m_created = false;

    return *this;
  }

  bool CSubscriber::Create(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const Subscriber::Configuration& config_)
  {
    if (m_created)           return(false);
    if (topic_name_.empty()) return(false);

    // create datareader
    m_datareader = std::make_shared<CDataReader>(topic_name_, data_type_info_);

    // register datareader
    g_subgate()->Register(topic_name_, m_datareader);

    // we made it :-)
    m_created = true;
    return(m_created);
  }

  bool CSubscriber::Create(const std::string& topic_name_)
  {
    return Create(topic_name_, SDataTypeInformation{});
  }

  bool CSubscriber::Destroy()
  {
    if(!m_created) return(false);

    // remove receive callback
    RemReceiveCallback();

    // unregister datareader
    if(g_subgate() != nullptr) g_subgate()->Unregister(m_datareader->GetTopicName(), m_datareader);

#ifndef NDEBUG
    // log it
    if (g_log() != nullptr) g_log()->Log(log_level_debug1, std::string(m_datareader->GetTopicName() + "::CSubscriber::Destroy"));
#endif

    // stop & destroy datareader
    m_datareader->Stop();
    m_datareader.reset();

    // we made it :-)
    m_created = false;

    return(true);
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

  bool CSubscriber::ReceiveBuffer(std::string& buf_, long long* time_ /* = nullptr */, int rcv_timeout_ /* = 0 */) const
  {
    if (!m_created) return(false);
    return(m_datareader->Receive(buf_, time_, rcv_timeout_));
  }

  bool CSubscriber::AddReceiveCallback(ReceiveCallbackT callback_)
  {
    if(m_datareader == nullptr) return(false);
    RemReceiveCallback();
    return(m_datareader->AddReceiveCallback(std::move(callback_)));
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
    if (m_datareader == nullptr) return(0);
    return(m_datareader->GetPublisherCount());
  }

  std::string CSubscriber::GetTopicName() const
  {
    if(m_datareader == nullptr) return("");
    return(m_datareader->GetTopicName());
  }
  
  SDataTypeInformation CSubscriber::GetDataTypeInformation() const
  {
    if (m_datareader == nullptr) return(SDataTypeInformation{});
    return(m_datareader->GetDataTypeInformation());
  }

  std::string CSubscriber::Dump(const std::string& indent_ /* = "" */) const
  {
    std::stringstream out;

    out << indent_ << "----------------------" << '\n';
    out << indent_ << " class CSubscriber    " << '\n';
    out << indent_ << "----------------------" << '\n';
    out << indent_ << "m_created:            " << m_created << '\n';
    if((m_datareader != nullptr) && m_datareader->IsCreated()) out << indent_ << m_datareader->Dump("    ");
    out << '\n';

    return(out.str());
  }
}
