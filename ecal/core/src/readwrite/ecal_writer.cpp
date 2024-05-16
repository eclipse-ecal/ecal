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
 * @brief  common eCAL data writer
**/

#include <ecal/ecal.h>
#include <ecal/ecal_config.h>
#include <ecal/ecal_payload_writer.h>
#include <functional>
#include <mutex>
#include <string>
#include <utility>

#include "config/ecal_config_reader_hlp.h"

#if ECAL_CORE_REGISTRATION
#include "registration/ecal_registration_provider.h"
#endif

#include "ecal_writer.h"
#include "ecal_writer_base.h"
#include "ecal_writer_buffer_payload.h"

#include "pubsub/ecal_pubgate.h"

#include <sstream>
#include <chrono>

struct SSndHash
{
  SSndHash(std::string t, long long c) : topic_id(t), snd_clock(c) {}
  std::string topic_id;
  long long   snd_clock;
};

namespace std
{
  template<>
  class hash<SSndHash> {
  public:
    size_t operator()(const SSndHash& h) const
    {
      const size_t h1 = std::hash<std::string>()(h.topic_id);
      const size_t h2 = std::hash<long long>()(h.snd_clock);
      return h1 ^ (h2 << 1);
    }
  };
}

namespace eCAL
{
  CDataWriter::CDataWriter(const std::string& topic_name_, const SDataTypeInformation& topic_info_, const Publisher::Configuration& config_) :
    m_host_name(Process::GetHostName()),
    m_host_group_name(Process::GetHostGroupName()),
    m_pid(Process::GetProcessID()),
    m_pname(Process::GetProcessName()),
    m_topic_name(topic_name_),
    m_topic_info(topic_info_),
    m_config(config_),
    m_connected(false),
    m_frequency_calculator(3.0f),
    m_created(false)
  {
#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug1, m_topic_name + "::CDataWriter::Constructor");
#endif

    // build topic id
    std::stringstream counter;
    counter << std::chrono::steady_clock::now().time_since_epoch().count();
    m_topic_id = counter.str();

    // set registration expiration
    const std::chrono::milliseconds registration_timeout(Config::GetRegistrationTimeoutMs());
    m_sub_map.set_expiration(registration_timeout);

    // mark as created
    m_created = true;

    // register
    Register(false);

    // start udp, shm, tcp layer
    StartTransportLayer();
  }

  CDataWriter::~CDataWriter()
  {
#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug1, m_topic_name + "::CDataWriter::Destructor");
#endif

    Stop();
  }

  bool CDataWriter::Stop()
  {
    if (!m_created) return false;
#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug1, m_topic_name + "::CDataWriter::Stop");
#endif

    // stop udp, shm, tcp layer
    StopTransportLayer();

    // clear subscriber maps
    {
      const std::lock_guard<std::mutex> lock(m_sub_map_mtx);
      m_sub_map.clear();
    }

    // clear event callback map
    {
      const std::lock_guard<std::mutex> lock(m_event_callback_map_mtx);
      m_event_callback_map.clear();
    }

    // mark as no more created
    m_created = false;

    // and unregister
    Unregister();

    return true;
  }

  bool CDataWriter::SetDataTypeInformation(const SDataTypeInformation& topic_info_)
  {
    // Does it even make sense to register if the info is the same???
    const bool force = m_topic_info != topic_info_;
    m_topic_info = topic_info_;

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug2, m_topic_name + "::CDataWriter::SetDescription");
#endif

    // register it
    Register(force);

    return(true);
  }

  bool CDataWriter::SetAttribute(const std::string& attr_name_, const std::string& attr_value_)
  {
    auto current_val = m_attr.find(attr_name_);

    const bool force = current_val == m_attr.end() || current_val->second != attr_value_;
    m_attr[attr_name_] = attr_value_;

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug2, m_topic_name + "::CDataWriter::SetAttribute");
#endif

    // register it
    Register(force);

    return(true);
  }

  bool CDataWriter::ClearAttribute(const std::string& attr_name_)
  {
    auto force = m_attr.find(attr_name_) != m_attr.end();

    m_attr.erase(attr_name_);

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug2, m_topic_name + "::CDataWriter::ClearAttribute");
#endif

    // register it
    Register(force);

    return(true);
  }

  bool CDataWriter::AddEventCallback(eCAL_Publisher_Event type_, PubEventCallbackT callback_)
  {
    if (!m_created) return(false);

    // store event callback
    {
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug2, m_topic_name + "::CDataWriter::AddEventCallback");
#endif
      const std::lock_guard<std::mutex> lock(m_event_callback_map_mtx);
      m_event_callback_map[type_] = std::move(callback_);
    }

    return(true);
  }

  bool CDataWriter::RemEventCallback(eCAL_Publisher_Event type_)
  {
    if (!m_created) return(false);

    // reset event callback
    {
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug2, m_topic_name + "::CDataWriter::RemEventCallback");
#endif
      const std::lock_guard<std::mutex> lock(m_event_callback_map_mtx);
      m_event_callback_map[type_] = nullptr;
    }

    return(true);
  }

  size_t CDataWriter::Write(CPayloadWriter& payload_, long long time_, long long id_)
  {
    {
      // we should think about if we would like to potentially use the `time_` variable to tick with (but we would need the same base for checking incoming samples then....
      const auto send_time = std::chrono::steady_clock::now();
      const std::lock_guard<std::mutex> lock(m_frequency_calculator_mtx);
      m_frequency_calculator.addTick(send_time);
    }

    // get payload buffer size (one time, to avoid multiple computations)
    const size_t payload_buf_size(payload_.GetSize());

    // can we do a zero copy write ?
    const bool allow_zero_copy =
         m_config.shm.zero_copy_mode // zero copy mode activated by user
      && m_writer_shm                // shm layer active
      && !m_writer_udp               // udp layer inactive
      && !m_writer_tcp;              // tcp layer inactive

    // create a payload copy for all layer
    if (!allow_zero_copy)
    {
      m_payload_buffer.resize(payload_buf_size);
      payload_.WriteFull(m_payload_buffer.data(), m_payload_buffer.size());
    }

    // prepare counter and internal states
    const size_t snd_hash = PrepareWrite(id_, payload_buf_size);

    // did we write anything
    bool written(false);

    ////////////////////////////////////////////////////////////////////////////
    // SHM
    ////////////////////////////////////////////////////////////////////////////
#if ECAL_CORE_TRANSPORT_SHM
    if (m_writer_shm)
    {
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug3, m_topic_name + "::CDataWriter::Send::SHM");
#endif

      // send it
      bool shm_sent(false);
      {
        // fill writer data
        struct SWriterAttr wattr;
        wattr.len                    = payload_buf_size;
        wattr.id                     = m_id;
        wattr.clock                  = m_clock;
        wattr.hash                   = snd_hash;
        wattr.time                   = time_;
        wattr.zero_copy              = m_config.shm.zero_copy_mode;
        wattr.acknowledge_timeout_ms = m_config.shm.acknowledge_timeout_ms;

        // prepare send
        if (m_writer_shm->PrepareWrite(wattr))
        {
          // register new to update listening subscribers and rematch
          Register(true);
          Process::SleepMS(5);
        }

        // we are the only active layer, and we support zero copy -> we do a zero copy write via payload
        if (allow_zero_copy)
        {
          // write to shm layer (write content into the opened memory file without additional copy)
          shm_sent = m_writer_shm->Write(payload_, wattr);
        }
        // multiple layer are active -> we make a copy and use that one
        else
        {
          // wrap the buffer into a payload object
          CBufferPayloadWriter payload_buf(m_payload_buffer.data(), m_payload_buffer.size());
          // write to shm layer (write content into the opened memory file without additional copy)
          shm_sent = m_writer_shm->Write(payload_buf, wattr);
        }

        m_confirmed_layers.shm = true;
      }
      written |= shm_sent;

#ifndef NDEBUG
      // log it
      if (shm_sent)
      {
        Logging::Log(log_level_debug3, m_topic_name + "::CDataWriter::Send::SHM - SUCCESS");
      }
      else
      {
        Logging::Log(log_level_error, m_topic_name + "::CDataWriter::Send::SHM - FAILED");
      }
#endif
    }
#endif // ECAL_CORE_TRANSPORT_SHM

    ////////////////////////////////////////////////////////////////////////////
    // UDP (MC)
    ////////////////////////////////////////////////////////////////////////////
#if ECAL_CORE_TRANSPORT_UDP
    if (m_writer_udp)
    {
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug3, m_topic_name + "::CDataWriter::Send::udp");
#endif

      // send it
      bool udp_sent(false);
      {
        // fill writer data
        struct SWriterAttr wattr;
        wattr.len       = payload_buf_size;
        wattr.id        = m_id;
        wattr.clock     = m_clock;
        wattr.hash      = snd_hash;
        wattr.time      = time_;
        wattr.loopback  = m_config.udp.loopback;

        // prepare send
        if (m_writer_udp->PrepareWrite(wattr))
        {
          // register new to update listening subscribers and rematch
          Register(true);
          Process::SleepMS(5);
        }

        // write to udp multicast layer
        udp_sent = m_writer_udp->Write(m_payload_buffer.data(), wattr);
        m_confirmed_layers.udp = true;
      }
      written |= udp_sent;

#ifndef NDEBUG
      // log it
      if (udp_sent)
      {
        Logging::Log(log_level_debug3, m_topic_name + "::CDataWriter::Send::udp - SUCCESS");
      }
      else
      {
        Logging::Log(log_level_error, m_topic_name + "::CDataWriter::Send::udp - FAILED");
      }
#endif
    }
#endif // ECAL_CORE_TRANSPORT_UDP

    ////////////////////////////////////////////////////////////////////////////
    // TCP
    ////////////////////////////////////////////////////////////////////////////
#if ECAL_CORE_TRANSPORT_TCP
    if (m_writer_tcp)
    {
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug3, m_topic_name + "::CDataWriter::Send::TCP");
#endif

      // send it
      bool tcp_sent(false);
      {
        // fill writer data
        struct SWriterAttr wattr;
        wattr.len   = payload_buf_size;
        wattr.id    = m_id;
        wattr.clock = m_clock;
        wattr.hash  = snd_hash;
        wattr.time  = time_;

        // write to tcp layer
        tcp_sent = m_writer_tcp->Write(m_payload_buffer.data(), wattr);
        m_confirmed_layers.tcp = true;
      }
      written |= tcp_sent;

#ifndef NDEBUG
      // log it
      if (tcp_sent)
      {
        Logging::Log(log_level_debug3, m_topic_name + "::CDataWriter::Send::TCP - SUCCESS");
      }
      else
      {
        Logging::Log(log_level_error, m_topic_name + "::CDataWriter::Send::TCP - FAILED");
      }
#endif
    }
#endif // ECAL_CORE_TRANSPORT_TCP

    // return success
    if (written) return payload_buf_size;
    else         return 0;
  }

  void CDataWriter::ApplySubscription(const SSubscriptionInfo& subscription_info_, const SDataTypeInformation& data_type_info_, const SLayerStates& layer_states_, const std::string& reader_par_)
  {
    Connect(subscription_info_.topic_id, data_type_info_);

    // add key to subscriber map
    {
      const std::lock_guard<std::mutex> lock(m_sub_map_mtx);
      m_sub_map[subscription_info_] = std::make_tuple(data_type_info_, layer_states_);
    }

    // add a new subscription
#if ECAL_CORE_TRANSPORT_UDP
    if (m_writer_udp) m_writer_udp->ApplySubscription(subscription_info_.host_name, subscription_info_.process_id, subscription_info_.topic_id, reader_par_);
#endif
#if ECAL_CORE_TRANSPORT_SHM
    if (m_writer_shm) m_writer_shm->ApplySubscription(subscription_info_.host_name, subscription_info_.process_id, subscription_info_.topic_id, reader_par_);
#endif
#if ECAL_CORE_TRANSPORT_TCP
    if (m_writer_tcp) m_writer_tcp->ApplySubscription(subscription_info_.host_name, subscription_info_.process_id, subscription_info_.topic_id, reader_par_);
#endif

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug3, m_topic_name + "::CDataWriter::ApplySubscription");
#endif
  }

  void CDataWriter::RemoveSubscription(const SSubscriptionInfo& subscription_info_)
  {
    // remove key from subscriber map
    {
      const std::lock_guard<std::mutex> lock(m_sub_map_mtx);
      m_sub_map.erase(subscription_info_);
    }

    // remove subscription
#if ECAL_CORE_TRANSPORT_UDP
    if (m_writer_udp) m_writer_udp->RemoveSubscription(subscription_info_.host_name, subscription_info_.process_id, subscription_info_.topic_id);
#endif
#if ECAL_CORE_TRANSPORT_SHM
    if (m_writer_shm) m_writer_shm->RemoveSubscription(subscription_info_.host_name, subscription_info_.process_id, subscription_info_.topic_id);
#endif
#if ECAL_CORE_TRANSPORT_TCP
    if (m_writer_tcp) m_writer_tcp->RemoveSubscription(subscription_info_.host_name, subscription_info_.process_id, subscription_info_.topic_id);
#endif

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug3, m_topic_name + "::CDataWriter::RemoveSubscription");
#endif
  }

  void CDataWriter::RefreshRegistration()
  {
    if (!m_created) return;

    // register without send
    Register(false);

    // check connection timeouts
    {
      const std::lock_guard<std::mutex> lock(m_sub_map_mtx);
      m_sub_map.remove_deprecated();

      if (m_sub_map.empty())
      {
        Disconnect();
      }
    }
  }

  void CDataWriter::RefreshSendCounter()
  {
    // increase write clock
    m_clock++;
  }

  std::string CDataWriter::Dump(const std::string& indent_ /* = "" */)
  {
    std::stringstream out;

    out << '\n';
    out << indent_ << "--------------------------" << '\n';
    out << indent_ << " class CDataWriter        " << '\n';
    out << indent_ << "--------------------------" << '\n';
    out << indent_ << "m_host_name:              " << m_host_name << '\n';
    out << indent_ << "m_host_group_name:        " << m_host_group_name << '\n';
    out << indent_ << "m_topic_name:             " << m_topic_name << '\n';
    out << indent_ << "m_topic_id:               " << m_topic_id << '\n';
    out << indent_ << "m_topic_info.encoding:    " << m_topic_info.encoding << '\n';
    out << indent_ << "m_topic_info.name:        " << m_topic_info.name << '\n';
    out << indent_ << "m_topic_info.desc:        " << m_topic_info.descriptor << '\n';
    out << indent_ << "m_id:                     " << m_id << '\n';
    out << indent_ << "m_clock:                  " << m_clock << '\n';
    out << indent_ << "frequency [mHz]:          " << GetFrequency() << '\n';
    out << indent_ << "m_created:                " << m_created << '\n';
    out << std::endl;

    return(out.str());
  }

  bool CDataWriter::Register(bool force_)
  {
#if ECAL_CORE_REGISTRATION
    if (!m_created)           return(false);
    if (m_topic_name.empty()) return(false);

    // create command parameter
    Registration::Sample ecal_reg_sample;
    ecal_reg_sample.cmd_type = bct_reg_publisher;

    auto& ecal_reg_sample_topic = ecal_reg_sample.topic;
    ecal_reg_sample_topic.hname  = m_host_name;
    ecal_reg_sample_topic.hgname = m_host_group_name;
    ecal_reg_sample_topic.tname  = m_topic_name;
    ecal_reg_sample_topic.tid    = m_topic_id;
    // topic_information
    {
      auto& ecal_reg_sample_tdatatype = ecal_reg_sample_topic.tdatatype;
      if (m_config.share_topic_type)
      {
        ecal_reg_sample_tdatatype.encoding   = m_topic_info.encoding;
        ecal_reg_sample_tdatatype.name       = m_topic_info.name;
      }
      if (m_config.share_topic_description)
      {
        ecal_reg_sample_tdatatype.descriptor = m_topic_info.descriptor;
      }
    }
    ecal_reg_sample_topic.attr  = m_attr;
    ecal_reg_sample_topic.tsize = static_cast<int32_t>(m_topic_size);

#if ECAL_CORE_TRANSPORT_UDP
    // udp multicast layer
    if (m_writer_udp)
    {
      eCAL::Registration::TLayer udp_tlayer;
      udp_tlayer.type                      = tl_ecal_udp;
      udp_tlayer.version                   = 1;
      udp_tlayer.confirmed                 = m_confirmed_layers.udp;
      udp_tlayer.par_layer.layer_par_udpmc = m_writer_udp->GetConnectionParameter().layer_par_udpmc;
      ecal_reg_sample_topic.tlayer.push_back(udp_tlayer);
    }
#endif

#if ECAL_CORE_TRANSPORT_SHM
    // shm layer
    if (m_writer_shm)
    {
      eCAL::Registration::TLayer shm_tlayer;
      shm_tlayer.type                    = tl_ecal_shm;
      shm_tlayer.version                 = 1;
      shm_tlayer.confirmed               = m_confirmed_layers.shm;
      shm_tlayer.par_layer.layer_par_shm = m_writer_shm->GetConnectionParameter().layer_par_shm;
      ecal_reg_sample_topic.tlayer.push_back(shm_tlayer);
    }
#endif

#if ECAL_CORE_TRANSPORT_TCP
    // tcp layer
    if (m_writer_tcp)
    {
      eCAL::Registration::TLayer tcp_tlayer;
      tcp_tlayer.type                    = tl_ecal_tcp;
      tcp_tlayer.version                 = 1;
      tcp_tlayer.confirmed               = m_confirmed_layers.tcp;
      tcp_tlayer.par_layer.layer_par_tcp = m_writer_tcp->GetConnectionParameter().layer_par_tcp;
      ecal_reg_sample_topic.tlayer.push_back(tcp_tlayer);
    }
#endif

    ecal_reg_sample_topic.pid    = m_pid;
    ecal_reg_sample_topic.pname  = m_pname;
    ecal_reg_sample_topic.uname  = Process::GetUnitName();
    ecal_reg_sample_topic.did    = m_id;
    ecal_reg_sample_topic.dclock = m_clock;
    ecal_reg_sample_topic.dfreq  = GetFrequency();

    size_t loc_connections(0);
    size_t ext_connections(0);
    {
      const std::lock_guard<std::mutex> lock(m_sub_map_mtx);
      for (const auto& sub : m_sub_map)
      {
        if (sub.first.host_name == m_host_name)
        {
          loc_connections++;
        }
      }
      ext_connections = m_sub_map.size() - loc_connections;
    }
    ecal_reg_sample_topic.connections_loc = static_cast<int32_t>(loc_connections);
    ecal_reg_sample_topic.connections_ext = static_cast<int32_t>(ext_connections);

    // register publisher
    if (g_registration_provider() != nullptr) g_registration_provider()->ApplySample(ecal_reg_sample, force_);

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug4, m_topic_name + "::CDataWriter::Register");
#endif

#endif // ECAL_CORE_REGISTRATION
    return(true);
  }

  bool CDataWriter::Unregister()
  {
#if ECAL_CORE_REGISTRATION
    if (m_topic_name.empty()) return(false);

    // create command parameter
    Registration::Sample ecal_unreg_sample;
    ecal_unreg_sample.cmd_type = bct_unreg_publisher;

    auto& ecal_reg_sample_topic  = ecal_unreg_sample.topic;
    ecal_reg_sample_topic.hname  = m_host_name;
    ecal_reg_sample_topic.hgname = m_host_group_name;
    ecal_reg_sample_topic.pname  = m_pname;
    ecal_reg_sample_topic.pid    = m_pid;
    ecal_reg_sample_topic.tname  = m_topic_name;
    ecal_reg_sample_topic.tid    = m_topic_id;
    ecal_reg_sample_topic.uname  = Process::GetUnitName();

    // unregister publisher
    if (g_registration_provider() != nullptr) g_registration_provider()->ApplySample(ecal_unreg_sample, false);

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug4, m_topic_name + "::CDataWriter::UnRegister");
#endif

#endif // ECAL_CORE_REGISTRATION
    return(true);
  }

  void CDataWriter::Connect(const std::string& tid_, const SDataTypeInformation& tinfo_)
  {
    SPubEventCallbackData data;
    data.time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    data.clock = 0;

    if (!m_connected)
    {
      m_connected = true;

      // fire pub_event_connected
      {
        const std::lock_guard<std::mutex> lock(m_event_callback_map_mtx);
        auto iter = m_event_callback_map.find(pub_event_connected);
        if (iter != m_event_callback_map.end() && iter->second)
        {
          data.type      = pub_event_connected;
          data.tid       = tid_;
          data.tdatatype = tinfo_;
          (iter->second)(m_topic_name.c_str(), &data);
        }
      }
    }

    // fire pub_event_update_connection
    {
      const std::lock_guard<std::mutex> lock(m_event_callback_map_mtx);
      auto iter = m_event_callback_map.find(pub_event_update_connection);
      if (iter != m_event_callback_map.end() && iter->second)
      {
        data.type      = pub_event_update_connection;
        data.tid       = tid_;
        data.tdatatype = tinfo_;
        (iter->second)(m_topic_name.c_str(), &data);
      }
    }
  }

  void CDataWriter::Disconnect()
  {
    if (m_connected)
    {
      m_connected = false;

      // fire pub_event_disconnected
      {
        const std::lock_guard<std::mutex> lock(m_event_callback_map_mtx);
        auto iter = m_event_callback_map.find(pub_event_disconnected);
        if (iter != m_event_callback_map.end() && iter->second)
        {
          SPubEventCallbackData data;
          data.type  = pub_event_disconnected;
          data.time  = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
          data.clock = 0;
          (iter->second)(m_topic_name.c_str(), &data);
        }
      }
    }
  }

  void CDataWriter::StartTransportLayer()
  {
#if ECAL_CORE_TRANSPORT_UDP
    if (m_config.udp.enable)
    {
      ActivateUdpLayer();
    }
#endif
#if ECAL_CORE_TRANSPORT_SHM
    if (m_config.shm.enable)
    {
      ActivateShmLayer();
    }
#endif
#if ECAL_CORE_TRANSPORT_TCP
    if (m_config.tcp.enable)
    {
      ActivateTcpLayer();
    }
#endif
  }

  void CDataWriter::StopTransportLayer()
  {
    // destroy udp writer
#if ECAL_CORE_TRANSPORT_UDP
    m_writer_udp.reset();
#endif

    // destroy shm writer
#if ECAL_CORE_TRANSPORT_SHM
    m_writer_shm.reset();
#endif

    // destroy tcp writer
#if ECAL_CORE_TRANSPORT_TCP
    m_writer_tcp.reset();
#endif
  }

  void CDataWriter::ActivateUdpLayer()
  {
#if ECAL_CORE_TRANSPORT_UDP
    // log state
    Logging::Log(log_level_debug4, m_topic_name + "::CDataWriter::ActivateUdpLayer::ACTIVATED");

    // create writer
    m_writer_udp = std::make_unique<CDataWriterUdpMC>(m_host_name, m_topic_name, m_topic_id, m_config.udp);

#ifndef NDEBUG
    Logging::Log(log_level_debug4, m_topic_name + "::CDataWriter::ActivateUdpLayer::WRITER_CREATED");
#endif
#endif // ECAL_CORE_TRANSPORT_UDP
  }

  void CDataWriter::ActivateShmLayer()
  {
#if ECAL_CORE_TRANSPORT_SHM
    // log state
    Logging::Log(log_level_debug4, m_topic_name + "::CDataWriter::ActivateShmLayer::ACTIVATED");

    // create writer
    m_writer_shm = std::make_unique<CDataWriterSHM>(m_host_name, m_topic_name, m_topic_id, m_config.shm);

#ifndef NDEBUG
    Logging::Log(log_level_debug4, m_topic_name + "::CDataWriter::ActivateShmLayer::WRITER_CREATED");
#endif
#endif // ECAL_CORE_TRANSPORT_SHM
  }

  void CDataWriter::ActivateTcpLayer()
  {
#if ECAL_CORE_TRANSPORT_TCP
    // log state
    Logging::Log(log_level_debug4, m_topic_name + "::CDataWriter::ActivateTcpLayer::ACTIVATED");

    // create writer
    m_writer_tcp = std::make_unique<CDataWriterTCP>(m_host_name, m_topic_name, m_topic_id, m_config.tcp);

#ifndef NDEBUG
    Logging::Log(log_level_debug4, m_topic_name + "::CDataWriter::ActivateTcpLayer::WRITER_CREATED");
#endif
#endif // ECAL_CORE_TRANSPORT_TCP
  }

  size_t CDataWriter::PrepareWrite(long long id_, size_t len_)
  {
    // store id
    m_id = id_;

    // handle write counters
    RefreshSendCounter();

    // calculate unique send hash
    const std::hash<SSndHash> hf;
    const size_t snd_hash = hf(SSndHash(m_topic_id, m_clock));

    // store size for monitoring
    m_topic_size = len_;

    // return the hash for the write action
    return snd_hash;
  }

  bool CDataWriter::IsInternalSubscribedOnly()
  {
    const int32_t process_id = static_cast<int32_t>(Process::GetProcessID());
    bool is_internal_only(true);
    const std::lock_guard<std::mutex> lock(m_sub_map_mtx);
    for (auto sub : m_sub_map)
    {
      if (sub.first.process_id != process_id)
      {
        is_internal_only = false;
        break;
      }
    }
    return is_internal_only;
  }

  int32_t CDataWriter::GetFrequency()
  {
    const auto frequency_time = std::chrono::steady_clock::now();
    const std::lock_guard<std::mutex> lock(m_frequency_calculator_mtx);
    return static_cast<int32_t>(m_frequency_calculator.getFrequency(frequency_time) * 1000);
  }
}
