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
#include <ecal/ecal_tlayer.h>

#include "ecal_def.h"
#include "ecal_config_hlp.h"
#include "ecal_globals.h"
#include "ecal_pubgate.h"
#include "ecal_register.h"

#include "readwrite/ecal_writer.h"

#include <sstream>
#include <iostream>

namespace eCAL
{
  CPublisher::CPublisher() :
                m_datawriter(nullptr),
                m_id(0),
                m_created(false),
                m_initialized(false)
  {
    InitializeQOS();
    InitializeTLayer();
  }

  CPublisher::CPublisher(const std::string& topic_name_, const std::string& topic_type_ /* = "" */, const std::string& topic_desc_ /* = "" */) :
                m_datawriter(nullptr),
                m_created(false),
                m_initialized(false)
  {
    InitializeQOS();
    InitializeTLayer();

    Create(topic_name_, topic_type_, topic_desc_);
  }

  CPublisher::~CPublisher()
  {
    Destroy();
  }

  /**
   * @brief CPublisher are move-enabled
  **/
  CPublisher::CPublisher(CPublisher&& rhs) noexcept :
                m_datawriter(std::move(rhs.m_datawriter)),
                m_qos(rhs.m_qos),
                m_id(rhs.m_id),
                m_created(rhs.m_created),
                m_initialized(rhs.m_initialized)
  {
    InitializeQOS();
    InitializeTLayer();

    rhs.m_created     = false;
    rhs.m_initialized = false;
  }

  /**
   * @brief CPublisher are move-enabled
  **/
  CPublisher& CPublisher::operator=(CPublisher&& rhs) noexcept
  {
    m_datawriter = std::move(rhs.m_datawriter);

    m_qos             = rhs.m_qos;
    m_id              = rhs.m_id;
    m_created         = rhs.m_created;
    m_initialized     = rhs.m_initialized;

    InitializeQOS();
    InitializeTLayer();
    rhs.m_created     = false;
    rhs.m_initialized = false;

    return *this;
  };

  bool CPublisher::Create(const std::string& topic_name_, const std::string& topic_type_ /* = "" */, const std::string& topic_desc_ /* = "" */)
  {
    if(m_created)               return(false);
    if(topic_name_.size() == 0) return(false);
    if(!g_globals())            return(false);

    // initialize globals
    if (!g_globals()->IsInitialized(Init::Publisher))
    {
      g_globals()->Initialize(Init::Publisher);
      m_initialized = true;
    }

    // check transport layer initialization
    // if layer API was not accessed before creation of this class
    // the layer mode will be initialized by the global config
    // this can not be done in the constructor because a publisher is allowed
    // to construct befor eCAL::Initialize and so global config is not
    // existing while construction
    if (m_tlayer.sm_udp_mc  == TLayer::smode_none) m_tlayer.sm_udp_mc = TLayer::eSendMode(eCALPAR(PUB, USE_UDP_MC));
    if (m_tlayer.sm_shm     == TLayer::smode_none) m_tlayer.sm_shm    = TLayer::eSendMode(eCALPAR(PUB, USE_SHM));
    if (m_tlayer.sm_tcp     == TLayer::smode_none) m_tlayer.sm_tcp    = TLayer::eSendMode(eCALPAR(PUB, USE_TCP));
    if (m_tlayer.sm_inproc  == TLayer::smode_none) m_tlayer.sm_inproc = TLayer::eSendMode(eCALPAR(PUB, USE_INPROC));

    // create data writer
    m_datawriter = new CDataWriter();
    // set qos
    m_datawriter->SetQOS(m_qos);
    // set transport layer
    m_datawriter->SetLayerMode(TLayer::tlayer_udp_mc,  m_tlayer.sm_udp_mc);
    m_datawriter->SetLayerMode(TLayer::tlayer_shm,     m_tlayer.sm_shm);
    m_datawriter->SetLayerMode(TLayer::tlayer_tcp,     m_tlayer.sm_tcp);
    m_datawriter->SetLayerMode(TLayer::tlayer_inproc,  m_tlayer.sm_inproc);
    // create it
    if (!m_datawriter->Create(topic_name_, topic_type_, topic_desc_))
    {
#ifndef NDEBUG
      // log it
      if (g_log()) g_log()->Log(log_level_debug1, topic_name_ + "::CPublisher::Create - FAILED");
#endif
      return(false);
    }
#ifndef NDEBUG
    // log it
    if (g_log()) g_log()->Log(log_level_debug1, topic_name_ + "::CPublisher::Create - SUCCESS");
#endif
    // register publisher gateway (for publisher memory file and event name)
    g_pubgate()->Register(topic_name_, m_datawriter);

    // register to description gateway for type / description checking
    g_descgate()->ApplyDescription(topic_name_, topic_type_, topic_desc_);

    // we made it :-)
    m_created = true;

    return(m_created);
  }

  bool CPublisher::Destroy()
  {
    if(!m_created)   return(false);
    if(!g_globals()) return(false);

    // destroy data writer
    m_datawriter->Destroy();

    // unregister data writer
    if(g_pubgate())         g_pubgate()->Unregister(m_datawriter->GetTopicName(), m_datawriter);
    if(g_entity_register()) g_entity_register()->UnregisterTopic(m_datawriter->GetTopicName(), m_datawriter->GetTopicID());
#ifndef NDEBUG
    // log it
    if (g_log()) g_log()->Log(log_level_debug1, std::string(m_datawriter->GetTopicName() + "::CPublisher::Destroy"));
#endif

    // free datawriter
    delete m_datawriter;
    m_datawriter = nullptr;

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

  bool CPublisher::SetDescription(const std::string& topic_desc_)
  {
    if(!m_datawriter) return false;
    return m_datawriter->SetDescription(topic_desc_);
  }

  bool CPublisher::SetAttribute(const std::string& attr_name_, const std::string& attr_value_)
  {
    if(!m_datawriter) return false;
    return m_datawriter->SetAttribute(attr_name_, attr_value_);
  }

  bool CPublisher::ClearAttribute(const std::string& attr_name_)
  {
    if(!m_datawriter) return false;
    return m_datawriter->ClearAttribute(attr_name_);
  }

  bool CPublisher::ShareType(bool state_ /*= true*/)
  {
    if (!m_datawriter) return false;
    m_datawriter->ShareType(state_);
    return true;
  }

  bool CPublisher::ShareDescription(bool state_ /*= true*/)
  {
    if (!m_datawriter) return false;
    m_datawriter->ShareDescription(state_);
    return true;
  }

  bool CPublisher::SetQOS(const QOS::SWriterQOS& qos_)
  {
    if (m_created) return false;
    m_qos = qos_;
    return true;
  }

  QOS::SWriterQOS CPublisher::GetQOS()
  {
    return m_qos;
  }

  bool CPublisher::SetLayerMode(TLayer::eTransportLayer layer_, TLayer::eSendMode mode_)
  {
    switch (layer_)
    {
    case TLayer::tlayer_udp_mc:
      m_tlayer.sm_udp_mc = mode_;
      break;
    case TLayer::tlayer_shm:
      m_tlayer.sm_shm = mode_;
      break;
    case TLayer::tlayer_tcp:
      m_tlayer.sm_tcp = mode_;
      break;
    case TLayer::tlayer_inproc:
      m_tlayer.sm_inproc = mode_;
      break;
    case TLayer::tlayer_all:
      m_tlayer.sm_udp_mc  = mode_;
      m_tlayer.sm_shm     = mode_;
      m_tlayer.sm_tcp     = mode_;
      m_tlayer.sm_inproc  = mode_;
      break;
    default:
      break;
    }
    if (m_created)
    {
      return m_datawriter->SetLayerMode(layer_, mode_);
    }
    return true;
  }

  bool CPublisher::SetMaxBandwidthUDP(long bandwidth_)
  {
    if (!m_created) return(false);
    return m_datawriter->SetMaxBandwidthUDP(bandwidth_);
  }

  bool CPublisher::ShmSetBufferCount(long buffering_)
  {
    if (!m_created) return(false);
    return m_datawriter->ShmSetBufferCount(buffering_);
  }

  bool CPublisher::ShmEnableZeroCopy(bool state_)
  {
    if (!m_created) return(false);
    return m_datawriter->ShmEnableZeroCopy(state_);
  }

  bool CPublisher::SetID(long long id_)
  {
    m_id = id_;
    return(true);
  }

  size_t CPublisher::Send(const void* const buf_, const size_t len_, const long long time_ /* = -1 */) const
  {
    if(!m_created) return(0);

    // in an optimization case the
    // publisher can send an empty package
    // or we do not have any subscription at all
    // then the data writer will only do some statistics
    // for the monitoring layer and return
    if (!IsSubscribed())
    {
      m_datawriter->RefreshSendCounter();
      return(len_);
    }

    // send content via data writer layer
    bool sent = 0;
    if (time_ == -1) sent = m_datawriter->Write(buf_, len_, eCAL::Time::GetMicroSeconds(), m_id);
    else             sent = m_datawriter->Write(buf_, len_, time_, m_id);

    // return success
    if (sent) return(len_);
    else      return(0);
  }

  size_t CPublisher::Send(const std::string& s_, const long long time_ /* = -1 */) const
  {
    return(Send(s_.data(), s_.size(), time_));
  }

  bool CPublisher::AddEventCallback(eCAL_Publisher_Event type_, PubEventCallbackT callback_)
  {
    if (!m_datawriter) return(false);
    RemEventCallback(type_);
    return(m_datawriter->AddEventCallback(type_, callback_));
  }

  bool CPublisher::RemEventCallback(eCAL_Publisher_Event type_)
  {
    if (!m_datawriter) return(false);
    return(m_datawriter->RemEventCallback(type_));
  }

  bool CPublisher::IsSubscribed() const
  {
    if(!m_datawriter) return(false);
    return(m_datawriter->IsSubscribed());
  }

  size_t CPublisher::GetSubscriberCount() const
  {
    if(!m_datawriter) return(0);
    return(m_datawriter->GetSubscriberCount());
  }

  std::string CPublisher::GetTopicName() const
  {
    if(!m_datawriter) return("");
    return(m_datawriter->GetTopicName());
  }

  std::string CPublisher::GetTypeName() const
  {
    if(!m_datawriter) return("");
    return(m_datawriter->GetTypeName());
  }

  std::string CPublisher::GetDescription() const
  {
    if(!m_datawriter) return("");
    return(m_datawriter->GetDescription());
  }

  void CPublisher::InitializeQOS()
  {
    m_qos = QOS::SWriterQOS();
  }

  void CPublisher::InitializeTLayer()
  {
    m_tlayer = TLayer::STLayer();
  }

  std::string CPublisher::Dump(const std::string& indent_ /* = "" */) const
  {
    std::stringstream out;

    out << indent_ << "----------------------" << std::endl;
    out << indent_ << " class CPublisher"      << std::endl;
    out << indent_ << "----------------------" << std::endl;
    out << indent_ << "m_created:            " << m_created << std::endl;
    if(m_datawriter && m_datawriter->IsCreated()) out << indent_ << m_datawriter->Dump("    ");
    out << std::endl;

    return(out.str());
  }
}
