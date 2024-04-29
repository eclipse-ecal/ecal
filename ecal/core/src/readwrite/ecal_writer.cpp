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
  CDataWriter::CDataWriter(const std::string& topic_name_, const SDataTypeInformation& topic_info_, const CPublisher::Config& config_) :
    m_host_name(Process::GetHostName()),
    m_host_group_name(Process::GetHostGroupName()),
    m_pid(Process::GetProcessID()),
    m_pname(Process::GetProcessName()),
    m_topic_name(topic_name_),
    m_topic_info(topic_info_),
    m_topic_size(0),
    m_config(config_),
    m_connected(false),
    m_id(0),
    m_clock(0),
    m_frequency_calculator(0.0f),
    m_loc_subscribed(false),
    m_ext_subscribed(false),
    m_created(false)
  {
    // shm config
#if ECAL_CORE_TRANSPORT_SHM
    m_writer.shm_mode.requested = config_.shm.send_mode;
#endif
    // udp config
#if ECAL_CORE_TRANSPORT_UDP
    m_writer.udp_mode.requested = config_.udp.send_mode;
#endif
    // tcp config
#if ECAL_CORE_TRANSPORT_TCP
    m_writer.tcp_mode.requested = config_.tcp.send_mode;
#endif

    // build topic id
    std::stringstream counter;
    counter << std::chrono::steady_clock::now().time_since_epoch().count();
    m_topic_id = counter.str();

    // set registration expiration
    const std::chrono::milliseconds registration_timeout(Config::GetRegistrationTimeoutMs());
    m_loc_sub_map.set_expiration(registration_timeout);
    m_ext_sub_map.set_expiration(registration_timeout);

    // mark as created
    m_created = true;

    // register
    Register(false);

    // create udp multicast layer
    SetUseUdpMC(m_writer.udp_mode.requested);

    // create shm layer
    SetUseShm(m_writer.shm_mode.requested);

    // create tcp layer
    SetUseTcp(m_writer.tcp_mode.requested);

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug1, m_topic_name + "::CDataWriter::Constructor");
#endif
  }

  CDataWriter::~CDataWriter()
  {
#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug1, m_topic_name + "::CDataWriter::Destructor");
#endif

    // destroy udp multicast writer
#if ECAL_CORE_TRANSPORT_UDP
    m_writer.udp.reset();
#endif

    // destroy memory file writer
#if ECAL_CORE_TRANSPORT_SHM
    m_writer.shm.reset();
#endif

    // destroy tcp writer
#if ECAL_CORE_TRANSPORT_TCP
    m_writer.tcp.reset();
#endif

    // clear subscriber maps
    {
      const std::lock_guard<std::mutex> lock(m_sub_map_sync);
      m_loc_sub_map.clear();
      m_ext_sub_map.clear();
    }

    // clear event callback map
    {
      const std::lock_guard<std::mutex> lock(m_event_callback_map_sync);
      m_event_callback_map.clear();
    }

    // mark as no more created
    m_created = false;

    // and unregister
    Unregister();
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
      const std::lock_guard<std::mutex> lock(m_event_callback_map_sync);
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
      const std::lock_guard<std::mutex> lock(m_event_callback_map_sync);
      m_event_callback_map[type_] = nullptr;
    }

    return(true);
  }

  size_t CDataWriter::Write(CPayloadWriter& payload_, long long time_, long long id_)
  {
    {
      // we should think about if we would like to potentially use the `time_` variable to tick with (but we would need the same base for checking incoming samples then....
      const auto send_time = std::chrono::steady_clock::now();
      const std::lock_guard<std::mutex> lock(m_frequency_calculator_mutex);
      m_frequency_calculator.addTick(send_time);
    }

    // check writer modes
    if (!CheckWriterModes())
    {
      // incompatible writer configurations
      return 0;
    }

    // get payload buffer size (one time, to avoid multiple computations)
    const size_t payload_buf_size(payload_.GetSize());

    // can we do a zero copy write ?
    const bool allow_zero_copy =
      m_config.shm.zero_copy_mode       // zero copy mode activated by user
   && m_writer.shm_mode.activated       // shm layer active
   && !m_writer.udp_mode.activated
   && !m_writer.tcp_mode.activated;

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
    if (m_writer.shm && m_writer.shm_mode.activated)
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
        wattr.buffering              = m_config.shm.memfile_buffer_count;
        wattr.zero_copy              = m_config.shm.zero_copy_mode;
        wattr.acknowledge_timeout_ms = m_config.shm.acknowledge_timeout_ms;

        // prepare send
        if (m_writer.shm->PrepareWrite(wattr))
        {
          // register new to update listening subscribers and rematch
          Register(true);
          Process::SleepMS(5);
        }

        // we are the only active layer, and we support zero copy -> we do a zero copy write via payload
        if (allow_zero_copy)
        {
          // write to shm layer (write content into the opened memory file without additional copy)
          shm_sent = m_writer.shm->Write(payload_, wattr);
        }
        // multiple layer are active -> we make a copy and use that one
        else
        {
          // wrap the buffer into a payload object
          CBufferPayloadWriter payload_buf(m_payload_buffer.data(), m_payload_buffer.size());
          // write to shm layer (write content into the opened memory file without additional copy)
          shm_sent = m_writer.shm->Write(payload_buf, wattr);
        }

        m_writer.shm_mode.confirmed = true;
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
    if (m_writer.udp && m_writer.udp_mode.activated)
    {
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug3, m_topic_name + "::CDataWriter::Send::udp");
#endif

      // send it
      bool udp_sent(false);
      {
#if ECAL_CORE_TRANSPORT_SHM
        // if shared memory layer for local communication is switched off
        // we activate udp message loopback to communicate with local processes too
        const bool loopback = m_writer.shm_mode.requested == TLayer::smode_off;
#else
        const bool loopback = true;
#endif

        // fill writer data
        struct SWriterAttr wattr;
        wattr.len       = payload_buf_size;
        wattr.id        = m_id;
        wattr.clock     = m_clock;
        wattr.hash      = snd_hash;
        wattr.time      = time_;
        wattr.loopback  = loopback;

        // prepare send
        if (m_writer.udp->PrepareWrite(wattr))
        {
          // register new to update listening subscribers and rematch
          Register(true);
          Process::SleepMS(5);
        }

        // write to udp multicast layer
        udp_sent = m_writer.udp->Write(m_payload_buffer.data(), wattr);
        m_writer.udp_mode.confirmed = true;
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
    if (m_writer.tcp && m_writer.tcp_mode.activated)
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
        tcp_sent = m_writer.tcp->Write(m_payload_buffer.data(), wattr);
        m_writer.tcp_mode.confirmed = true;
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

  void CDataWriter::ApplyLocSubscription(const SLocalSubscriptionInfo& local_info_, const SDataTypeInformation& tinfo_, const std::string& reader_par_)
  {
    Connect(local_info_.topic_id, tinfo_);

    // add key to local subscriber map
    {
      const std::lock_guard<std::mutex> lock(m_sub_map_sync);
      m_loc_sub_map[local_info_] = true;
    }

    m_loc_subscribed = true;

    // add a new local subscription
#if ECAL_CORE_TRANSPORT_UDP
    if (m_writer.udp) m_writer.udp->AddLocConnection(local_info_.process_id, local_info_.topic_id, reader_par_);
#endif
#if ECAL_CORE_TRANSPORT_SHM
    if (m_writer.shm) m_writer.shm->AddLocConnection(local_info_.process_id, local_info_.topic_id, reader_par_);
#endif
#if ECAL_CORE_TRANSPORT_TCP
    if (m_writer.tcp) m_writer.tcp->AddLocConnection(local_info_.process_id, local_info_.topic_id, reader_par_);
#endif

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug3, m_topic_name + "::CDataWriter::ApplyLocSubscription");
#endif
  }

  void CDataWriter::RemoveLocSubscription(const SLocalSubscriptionInfo& local_info_)
  {
    // remove key from local subscriber map
    {
      const std::lock_guard<std::mutex> lock(m_sub_map_sync);
      m_loc_sub_map.erase(local_info_);
    }

    // remove a local subscription
#if ECAL_CORE_TRANSPORT_UDP
    if (m_writer.udp) m_writer.udp->RemLocConnection(local_info_.process_id, local_info_.topic_id);
#endif
#if ECAL_CORE_TRANSPORT_SHM
    if (m_writer.shm) m_writer.shm->RemLocConnection(local_info_.process_id, local_info_.topic_id);
#endif
#if ECAL_CORE_TRANSPORT_TCP
    if (m_writer.tcp) m_writer.tcp->RemLocConnection(local_info_.process_id, local_info_.topic_id);
#endif

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug3, m_topic_name + "::CDataWriter::RemoveLocSubscription");
#endif
  }

  void CDataWriter::ApplyExtSubscription(const SExternalSubscriptionInfo& external_info_, const SDataTypeInformation& tinfo_, const std::string& reader_par_)
  {
    Connect(external_info_.topic_id, tinfo_);

    // add key to external subscriber map
    {
      const std::lock_guard<std::mutex> lock(m_sub_map_sync);
      m_ext_sub_map[external_info_] = true;
    }

    m_ext_subscribed = true;

    // add a new external subscription
#if ECAL_CORE_TRANSPORT_UDP
    if (m_writer.udp) m_writer.udp->AddExtConnection(external_info_.host_name, external_info_.process_id, external_info_.topic_id, reader_par_);
#endif
#if ECAL_CORE_TRANSPORT_SHM
    if (m_writer.shm) m_writer.shm->AddExtConnection(external_info_.host_name, external_info_.process_id, external_info_.topic_id, reader_par_);
#endif
#if ECAL_CORE_TRANSPORT_TCP
    if (m_writer.tcp) m_writer.tcp->AddExtConnection(external_info_.host_name, external_info_.process_id, external_info_.topic_id, reader_par_);
#endif

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug3, m_topic_name + "::CDataWriter::ApplyExtSubscription");
#endif
  }

  void CDataWriter::RemoveExtSubscription(const SExternalSubscriptionInfo& external_info_)
  {
    // remove key from external subscriber map
    {
      const std::lock_guard<std::mutex> lock(m_sub_map_sync);
      m_ext_sub_map.erase(external_info_);
    }

    // remove external subscription
#if ECAL_CORE_TRANSPORT_UDP
    if (m_writer.udp) m_writer.udp->RemExtConnection(external_info_.host_name, external_info_.process_id, external_info_.topic_id);
#endif
#if ECAL_CORE_TRANSPORT_SHM
    if (m_writer.shm) m_writer.shm->RemExtConnection(external_info_.host_name, external_info_.process_id, external_info_.topic_id);
#endif
#if ECAL_CORE_TRANSPORT_TCP
    if (m_writer.tcp) m_writer.tcp->RemExtConnection(external_info_.host_name, external_info_.process_id, external_info_.topic_id);
#endif
  }

  void CDataWriter::RefreshRegistration()
  {
    if (!m_created) return;

    // register without send
    Register(false);

    // check connection timeouts
    {
      const std::lock_guard<std::mutex> lock(m_sub_map_sync);
      m_loc_sub_map.remove_deprecated();
      m_ext_sub_map.remove_deprecated();

      m_loc_subscribed = !m_loc_sub_map.empty();
      m_ext_subscribed = !m_ext_sub_map.empty();
    }

    if (!m_loc_subscribed && !m_ext_subscribed)
    {
      Disconnect();
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
    out << indent_ << "m_loc_subscribed:         " << m_loc_subscribed << '\n';
    out << indent_ << "m_ext_subscribed:         " << m_ext_subscribed << '\n';
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
    if (m_writer.udp)
    {
      eCAL::Registration::TLayer udp_tlayer;
      udp_tlayer.type                      = tl_ecal_udp_mc;
      udp_tlayer.version                   = 1;
      udp_tlayer.confirmed                 = m_writer.udp_mode.confirmed;
      udp_tlayer.par_layer.layer_par_udpmc = m_writer.udp->GetConnectionParameter().layer_par_udpmc;
      ecal_reg_sample_topic.tlayer.push_back(udp_tlayer);
    }
#endif

#if ECAL_CORE_TRANSPORT_SHM
    // shm layer
    if (m_writer.shm)
    {
      eCAL::Registration::TLayer shm_tlayer;
      shm_tlayer.type                    = tl_ecal_shm;
      shm_tlayer.version                 = 1;
      shm_tlayer.confirmed               = m_writer.shm_mode.confirmed;
      shm_tlayer.par_layer.layer_par_shm = m_writer.shm->GetConnectionParameter().layer_par_shm;
      ecal_reg_sample_topic.tlayer.push_back(shm_tlayer);
    }
#endif

#if ECAL_CORE_TRANSPORT_TCP
    // tcp layer
    if (m_writer.tcp)
    {
      eCAL::Registration::TLayer tcp_tlayer;
      tcp_tlayer.type                    = tl_ecal_tcp;
      tcp_tlayer.version                 = 1;
      tcp_tlayer.confirmed               = m_writer.tcp_mode.confirmed;
      tcp_tlayer.par_layer.layer_par_tcp = m_writer.tcp->GetConnectionParameter().layer_par_tcp;
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
      const std::lock_guard<std::mutex> lock(m_sub_map_sync);
      loc_connections = m_loc_sub_map.size();
      ext_connections = m_ext_sub_map.size();
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
        const std::lock_guard<std::mutex> lock(m_event_callback_map_sync);
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
      const std::lock_guard<std::mutex> lock(m_event_callback_map_sync);
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
        const std::lock_guard<std::mutex> lock(m_event_callback_map_sync);
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

  void CDataWriter::SetUseUdpMC(TLayer::eSendMode mode_)
  {
#if ECAL_CORE_TRANSPORT_UDP
    m_writer.udp_mode.requested = mode_;
    if (!m_created) return;

    // log send mode
    LogSendMode(mode_, m_topic_name + "::CDataWriter::Create::UDP_MC_SENDMODE::");

    switch (mode_)
    {
    case TLayer::eSendMode::smode_auto:
    case TLayer::eSendMode::smode_on:
      m_writer.udp = std::make_unique<CDataWriterUdpMC>(m_host_name, m_topic_name, m_topic_id, m_config.udp);
#ifndef NDEBUG
      Logging::Log(log_level_debug4, m_topic_name + "::CDataWriter::Create::UDP_MC_WRITER");
#endif
      break;
    case TLayer::eSendMode::smode_none:
    case TLayer::eSendMode::smode_off:
      m_writer.udp.reset();
      break;
    }
#endif // ECAL_CORE_TRANSPORT_UDP
  }

  void CDataWriter::SetUseShm(TLayer::eSendMode mode_)
  {
#if ECAL_CORE_TRANSPORT_SHM
    m_writer.shm_mode.requested = mode_;
    if (!m_created) return;

    // log send mode
    LogSendMode(mode_, m_topic_name + "::CDataWriter::Create::SHM_SENDMODE::");

    switch (mode_)
    {
    case TLayer::eSendMode::smode_auto:
    case TLayer::eSendMode::smode_on:
      m_writer.shm = std::make_unique<CDataWriterSHM>(m_host_name, m_topic_name, m_topic_id, m_config.shm);
      m_writer.shm->SetBufferCount(m_config.shm.memfile_buffer_count);
#ifndef NDEBUG
      Logging::Log(log_level_debug4, m_topic_name + "::CDataWriter::Create::SHM_WRITER");
#endif
      break;
    case TLayer::eSendMode::smode_none:
    case TLayer::eSendMode::smode_off:
      m_writer.shm.reset();
      break;
    }
#endif // ECAL_CORE_TRANSPORT_SHM
  }

  void CDataWriter::SetUseTcp(TLayer::eSendMode mode_)
  {
#if ECAL_CORE_TRANSPORT_TCP
    m_writer.tcp_mode.requested = mode_;
    if (!m_created) return;

    // log send mode
    LogSendMode(mode_, m_topic_name + "::CDataWriter::Create::TCP_SENDMODE::");

    switch (mode_)
    {
    case TLayer::eSendMode::smode_auto:
    case TLayer::eSendMode::smode_on:
      m_writer.tcp = std::make_unique<CDataWriterTCP>(m_host_name, m_topic_name, m_topic_id, m_config.tcp);
#ifndef NDEBUG
      Logging::Log(log_level_debug4, m_topic_name + "::CDataWriter::Create::TCP_WRITER - SUCCESS");
#endif
      break;
    case TLayer::eSendMode::smode_none:
    case TLayer::eSendMode::smode_off:
      m_writer.tcp.reset();
      break;
    }
#else // ECAL_CORE_TRANSPORT_TCP
    (void)mode_;
#endif // ECAL_CORE_TRANSPORT_TCP
  }

  bool CDataWriter::CheckWriterModes()
  {
    // if nothing is activated, we use defaults shm = auto, udp = auto
    if ((m_writer.udp_mode.requested == TLayer::smode_off)
     && (m_writer.shm_mode.requested == TLayer::smode_off)
     && (m_writer.tcp_mode.requested == TLayer::smode_off)
      )
    {
#if ECAL_CORE_TRANSPORT_UDP
      m_writer.udp_mode.requested = TLayer::smode_auto;
#endif
#if ECAL_CORE_TRANSPORT_SHM
      m_writer.shm_mode.requested = TLayer::smode_auto;
#endif
    }

    // if shm layer is off, we need a local transport layer switch to on
    // prio 1: udp = on
    // prio 2: tcp = on
    if ( (m_writer.shm_mode.requested == TLayer::smode_off)
      && (m_writer.shm_mode.requested != TLayer::smode_on)
      && (m_writer.tcp_mode.requested != TLayer::smode_on)
      )
    {
      bool new_local_layer(false);
#if ECAL_CORE_TRANSPORT_UDP
      if (m_writer.udp_mode.requested != TLayer::smode_on)
      {
        m_writer.udp_mode.requested = TLayer::smode_on;
        new_local_layer = true;
      }
#else
  #if ECAL_CORE_TRANSPORT_TCP
      if (m_writer.tcp_mode.requested != TLayer::smode_on)
      {
        m_writer.tcp_mode.requested = TLayer::smode_on;
        new_local_layer = true;
      }
#endif
#endif
      if (new_local_layer)
      {
        if (m_writer.udp_mode.requested == TLayer::smode_on)
        {
          Logging::Log(log_level_warning, m_topic_name + "::CDataWriter: Switched to udp for local communication.");
          SetUseUdpMC(TLayer::smode_on);
        }
        if (m_writer.tcp_mode.requested == TLayer::smode_on)
        {
          Logging::Log(log_level_warning, m_topic_name + "::CDataWriter: Switched to tcp for local communication.");
          SetUseTcp(TLayer::smode_on);
        }
      }
    }

    if ( (m_writer.tcp_mode.requested == TLayer::smode_auto)
      && (m_writer.udp_mode.requested == TLayer::smode_auto)
      )
    {
      Logging::Log(log_level_error, m_topic_name + "::CDataWriter::Send: TCP layer and UDP layer are both set to auto mode - Publication failed !");
      return false;
    }

#if ECAL_CORE_TRANSPORT_UDP
    ////////////////////////////////////////////////////////////////////////////
    // UDP (MC)
    ////////////////////////////////////////////////////////////////////////////
    if (((m_writer.udp_mode.requested == TLayer::smode_auto) && m_ext_subscribed)
      || (m_writer.udp_mode.requested == TLayer::smode_on)
      )
    {
      m_writer.udp_mode.activated = true;
    }
#endif

#if ECAL_CORE_TRANSPORT_SHM
    ////////////////////////////////////////////////////////////////////////////
    // SHM
    ////////////////////////////////////////////////////////////////////////////
    if (((m_writer.shm_mode.requested == TLayer::smode_auto) && m_loc_subscribed)
      || (m_writer.shm_mode.requested == TLayer::smode_on)
      )
    {
      m_writer.shm_mode.activated = true;
    }
#endif

#if ECAL_CORE_TRANSPORT_TCP
    ////////////////////////////////////////////////////////////////////////////
    // TCP
    ////////////////////////////////////////////////////////////////////////////
    if (((m_writer.tcp_mode.requested == TLayer::smode_auto) && m_ext_subscribed)
      || (m_writer.tcp_mode.requested == TLayer::smode_on)
      )
    {
      m_writer.tcp_mode.activated = true;
    }
#endif

    return true;
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
    const std::string process_id = Process::GetProcessIDAsString();
    bool is_internal_only(true);
    const std::lock_guard<std::mutex> lock(m_sub_map_sync);
    for (auto sub : m_loc_sub_map)
    {
      if (sub.first.process_id != process_id)
      {
        is_internal_only = false;
        break;
      }
    }
    return is_internal_only;
  }

  void CDataWriter::LogSendMode(TLayer::eSendMode smode_, const std::string& base_msg_)
  {
#ifndef NDEBUG
    switch (smode_)
    {
    case TLayer::eSendMode::smode_none:
      Logging::Log(log_level_debug4, base_msg_ + "NONE");
      break;
    case TLayer::eSendMode::smode_auto:
      Logging::Log(log_level_debug4, base_msg_ + "AUTO");
      break;
    case TLayer::eSendMode::smode_on:
      Logging::Log(log_level_debug4, base_msg_ + "ON");
      break;
    case TLayer::eSendMode::smode_off:
      Logging::Log(log_level_debug4, base_msg_ + "OFF");
      break;
    }
#else
    (void)smode_;
    (void)base_msg_;
#endif
  }
  int32_t CDataWriter::GetFrequency()
  {
    const auto frequency_time = std::chrono::steady_clock::now();
    const std::lock_guard<std::mutex> lock(m_frequency_calculator_mutex);
    return static_cast<int32_t>(m_frequency_calculator.getFrequency(frequency_time) * 1000);
  }
}
