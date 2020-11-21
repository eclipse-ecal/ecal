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

#include "ecal_def.h"
#include "ecal_config_hlp.h"
#include "ecal_reggate.h"
#include "ecal_writer.h"
#include "ecal_writer_base.h"

#include "ecal_register.h"
#include "pubsub/ecal_pubgate.h"

#include <sstream>
#include <chrono>
#include <functional>

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
    size_t operator()(const SSndHash &h) const
    {
      size_t h1 = std::hash<std::string>()(h.topic_id);
      size_t h2 = std::hash<long long>()(h.snd_clock);
      return h1 ^ (h2 << 1);
    }
  };
}

namespace eCAL
{
  CDataWriter::CDataWriter() :
    m_host_name(Process::GetHostName()),
    m_host_id(Process::GetHostID()),
    m_pid(Process::GetProcessID()),
    m_pname(Process::GetProcessName()),
    m_topic_size(0),
    m_connected(false),
    m_id(0),
    m_clock(0),
    m_clock_old(0),
    m_snd_time(),
    m_freq(0),
    m_bandwidth_max_udp(NET_BANDWIDTH_MAX_UDP),
    m_loc_subscribed(false),
    m_ext_subscribed(false),
    m_use_udp_mc(TLayer::eSendMode(eCALPAR(PUB, USE_UDP_MC))),
    m_use_udp_mc_confirmed(false),
    m_use_shm(TLayer::eSendMode(eCALPAR(PUB, USE_SHM))),
    m_use_shm_confirmed(false),
    m_use_inproc(TLayer::eSendMode(eCALPAR(PUB, USE_INPROC))),
    m_use_inproc_confirmed(false),
    m_use_ttype(true),
    m_use_tdesc(true),
    m_share_ttype(-1),
    m_share_tdesc(-1),
    m_created(false)
  {
  }

  CDataWriter::~CDataWriter()
  {
    Destroy();
  }

  bool CDataWriter::Create(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_)
  {
    if (m_created) return(false);

    // set defaults
    m_topic_name        = topic_name_;
    m_topic_id.clear();
    m_topic_type        = topic_type_;
    m_topic_desc        = topic_desc_;
    m_id                = 0;
    m_clock             = 0;
    m_clock_old         = 0;
    m_snd_time          = std::chrono::steady_clock::time_point();
    m_freq              = 0;
    m_bandwidth_max_udp = eCALPAR(NET, BANDWIDTH_MAX_UDP);
    m_ext_subscribed    = false;
    m_created           = false;

    // build topic id
    std::stringstream counter;
    counter << std::chrono::steady_clock::now().time_since_epoch().count();
    m_topic_id = counter.str();

    // set registration expiration
    std::chrono::milliseconds registration_timeout(eCALPAR(CMN, REGISTRATION_TO));
    m_loc_sub_map.set_expiration(registration_timeout);
    m_ext_sub_map.set_expiration(registration_timeout);

    // allow to share topic type
    m_use_ttype = eCALPAR(PUB, SHARE_TTYPE) != 0;

    // allow to share topic description
    m_use_tdesc = eCALPAR(PUB, SHARE_TDESC) != 0;

    // register now
    DoRegister(true);

    // mark as created
    m_created = true;

    // create udp multicast layer
    SetUseUdpMC(m_use_udp_mc);

    // create shm layer
    SetUseShm(m_use_shm);

    // create inproc layer
    SetUseInProc(m_use_inproc);

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug1, m_topic_name + "::CDataWriter::Created");
#endif

    return(true);
  }

  bool CDataWriter::Destroy()
  {
    if (!m_created) return(false);

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug1, m_topic_name + "::CDataWriter::Destroy");
#endif

    // destroy udp multicast writer
    m_writer_udp_mc.Destroy();

    // destroy memory file writer
    m_writer_shm.Destroy();

    // destroy inproc writer
    m_writer_inproc.Destroy();

    // reset defaults
    m_id                = 0;
    m_clock             = 0;
    m_clock_old         = 0;
    m_snd_time          = std::chrono::steady_clock::time_point();
    m_freq              = 0;
    m_bandwidth_max_udp = eCALPAR(NET, BANDWIDTH_MAX_UDP);
    m_created           = false;

    return(true);
  }

  bool CDataWriter::SetDescription(const std::string& topic_desc_)
  {
    bool force = m_topic_desc != topic_desc_;
    m_topic_desc = topic_desc_;

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug2, m_topic_name + "::CDataWriter::SetDescription");
#endif

    // register it
    DoRegister(force);

    return(true);
  }

  void CDataWriter::ShareType(bool state_)
  {
    if (state_)
    {
      m_share_ttype = 1;
    }
    else
    {
      m_share_ttype = 0;
    }
  }

  void CDataWriter::ShareDescription(bool state_)
  {
    if (state_)
    {
      m_share_tdesc = 1;
    }
    else
    {
      m_share_tdesc = 0;
    }
  }

  bool CDataWriter::SetQOS(const QOS::SWriterQOS& qos_)
  {
    m_qos = qos_;
    bool ret = true;
    ret &= m_writer_shm.SetQOS(qos_);
    return ret;
  }

  bool CDataWriter::SetLayerMode(TLayer::eTransportLayer layer_, TLayer::eSendMode mode_)
  {
    switch (layer_)
    {
    case TLayer::tlayer_udp_mc:
      SetUseUdpMC(mode_);
      break;
    case TLayer::tlayer_shm:
      SetUseShm(mode_);
      break;
    case TLayer::tlayer_inproc:
      SetUseInProc(mode_);
      break;
    case TLayer::tlayer_all:
      SetUseUdpMC   (mode_);
      SetUseShm     (mode_);
      SetUseInProc  (mode_);
      break;
    default:
      break;
    }
    return true;
  }

  bool CDataWriter::SetMaxBandwidthUDP(long bandwidth_)
  {
    m_bandwidth_max_udp = bandwidth_;
    return false;
  }

  bool CDataWriter::AddEventCallback(eCAL_Publisher_Event type_, PubEventCallbackT callback_)
  {
    if (!m_created) return(false);

    // store event callback
    {
      std::lock_guard<std::mutex> lock(m_event_callback_sync);
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug2, m_topic_name + "::CDataWriter::AddEventCallback");
#endif
      m_event_callback_map[type_] = callback_;
    }

    return(true);
  }

  bool CDataWriter::RemEventCallback(eCAL_Publisher_Event type_)
  {
    if (!m_created) return(false);

    // reset event callback
    {
      std::lock_guard<std::mutex> lock(m_event_callback_sync);
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug2, m_topic_name + "::CDataWriter::RemEventCallback");
#endif
      m_event_callback_map[type_] = nullptr;
    }

    return(true);
  }

  size_t CDataWriter::Send(const void* const buf_, size_t len_, long long time_, long long id_)
  {
    // store id
    m_id = id_;

    // handle write counters
    RefreshSendCounter();

    // calculate unique send hash
    std::hash<SSndHash> hf;
    size_t snd_hash = hf(SSndHash(m_topic_id, m_clock));

    // increase overall sum send
    g_process_wbytes_sum += len_;

    // store size for monitoring
    m_topic_size = len_;

    // did we write anything
    bool written(false);

    // check send modes
    TLayer::eSendMode use_udp_mc(m_use_udp_mc);
    TLayer::eSendMode use_shm(m_use_shm);
    TLayer::eSendMode use_inproc(m_use_inproc);
    if ( (use_udp_mc  == TLayer::smode_off)
      && (use_shm     == TLayer::smode_off)
      && (use_inproc  == TLayer::smode_off)
      )
    {
      // failsafe default mode if
      // nothing is activated
      use_udp_mc = TLayer::smode_auto;
      use_shm     = TLayer::smode_auto;
    }

    // if we do not have loopback
    // enabled we can switch off
    // inner process communication
    if (g_reggate() && !g_reggate()->LoopBackEnabled())
    {
      use_inproc = TLayer::smode_off;
    }
    
    // shared memory transport is on and
    // inner process transport is on
    // let's check if there is a need for
    // shared memory because of external
    // process subscription, if not
    // let's switch it off
    if  ((use_shm      != TLayer::smode_off)
      && (use_inproc   != TLayer::smode_off)
      )
    {
      if (!IsExtSubscribed())
      {
        // we have no external subscriptions,
        // but we have local ones (otherwise we would have
        // no subscriptions and this is checked with !IsSubscribed())
        // so let's check if all local subscriptions are 
        // "inner process only", that means
        // they have all our process id
        if (IsInternalSubscribedOnly())
        {
          // we can switch shared memory layer off
          // it's all subscribed in our process
          use_shm = TLayer::smode_off;
        }
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    // LAYER 1 : INPROC
    ////////////////////////////////////////////////////////////////////////////
    if  ((use_inproc == TLayer::smode_auto)
      || (use_inproc == TLayer::smode_on)
      )
    {
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug3, m_topic_name + "::CDataWriter::Send::InProc");
#endif

      // prepare send
      if (m_writer_inproc.PrepareSend(len_))
      {
        // register new to update listening subscribers
        DoRegister(true);
        // let's rematch writer / reader
        Process::SleepMS(5);
      }

      // send it
      size_t inproc_sent(0);
      {
        struct CDataWriterBase::SWriterData wdata;
        wdata.buf   = buf_;
        wdata.len   = len_;
        wdata.id    = m_id;
        wdata.clock = m_clock;
        wdata.hash  = snd_hash;
        wdata.time  = time_;
        inproc_sent = m_writer_inproc.Send(wdata);
        m_use_inproc_confirmed = true;
      }
      written |= inproc_sent > 0;

#ifndef NDEBUG
      // log it
      if (inproc_sent > 0)
      {
        Logging::Log(log_level_debug3, m_topic_name + "::CDataWriter::Send::InProc - SUCCESS");
      }
      else
      {
        // if "m_writer_inproc.Send" returns 0 it's not a fault, the inner process writer may have no
        // subscription and so it will return 0 written bytes
        // the other layers will write their bytes in any case on the specific layer
        // so we will not handle this as an error
      }
#endif
    }

    ////////////////////////////////////////////////////////////////////////////
    // LAYER 2 : SHM
    ////////////////////////////////////////////////////////////////////////////
    if (((use_shm == TLayer::smode_auto) && m_loc_subscribed)
      || (use_shm == TLayer::smode_on)
      )
    {
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug3, m_topic_name + "::CDataWriter::Send::MemFile");
#endif

      // prepare send
      if (m_writer_shm.PrepareSend(len_))
      {
        // register new to update listening subscribers
        DoRegister(true);
        // let's rematch writer / reader
        Process::SleepMS(5);
      }

      // send it
      size_t shm_sent(0);
      {
        struct CDataWriterBase::SWriterData wdata;
        wdata.buf   = buf_;
        wdata.len   = len_;
        wdata.id    = m_id;
        wdata.clock = m_clock;
        wdata.hash  = snd_hash;
        wdata.time  = time_;
        shm_sent    = m_writer_shm.Send(wdata);
        m_use_shm_confirmed = true;
      }
      written |= shm_sent > 0;

#ifndef NDEBUG
      // log it
      if (shm_sent > 0)
      {
        Logging::Log(log_level_debug3, m_topic_name + "::CDataWriter::Send::MemFile - SUCCESS");
      }
      else
      {
        Logging::Log(log_level_error, m_topic_name + "::CDataWriter::Send::MemFile - FAILED");
      }
#endif
    }

    ////////////////////////////////////////////////////////////////////////////
    // LAYER 3 : UDP (MC)
    ////////////////////////////////////////////////////////////////////////////
    if (((use_udp_mc == TLayer::smode_auto) && m_ext_subscribed)
      || (use_udp_mc == TLayer::smode_on)
      )
    {
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug3, m_topic_name + "::CDataWriter::Send::UDP_MC");
#endif

      // prepare send
      bool prepared(false);
      prepared = m_writer_udp_mc.PrepareSend(len_);

      if (prepared)
      {
        // register new to update listening subscribers
        DoRegister(true);
        // let's rematch writer / reader
        Process::SleepMS(5);
      }

      // send it
      size_t udp_mc_sent(0);
      {
        // if shared memory layer for local communication is switched off
        // we activate udp message loopback to communicate with local processes too
        bool loopback = use_shm == TLayer::smode_off;

        struct CDataWriterBase::SWriterData wdata;
        wdata.buf       = buf_;
        wdata.len       = len_;
        wdata.id        = m_id;
        wdata.clock     = m_clock;
        wdata.hash      = snd_hash;
        wdata.time      = time_;
        wdata.bandwidth = m_bandwidth_max_udp;
        wdata.loopback  = loopback;
        udp_mc_sent     = m_writer_udp_mc.Send(wdata);
        m_use_udp_mc_confirmed = true;
      }
      written |= udp_mc_sent > 0;

#ifndef NDEBUG
      // log it
      if (udp_mc_sent > 0)
      {
        Logging::Log(log_level_debug3, m_topic_name + "::CDataWriter::Send::UDP_MC - SUCCESS");
      }
      else
      {
        Logging::Log(log_level_error, m_topic_name + "::CDataWriter::Send::UDP_MC - FAILED");
      }
#endif
    }


    // return length if we succeeded
    if (!written) return(0);
    else          return(len_);
  }

  void CDataWriter::ApplyLocSubscription(const std::string& process_id_, const std::string& reader_par_)
  {
    SetConnected(true);
    {
      std::lock_guard<std::mutex> lock(m_sub_map_sync);
      m_loc_sub_map[process_id_] = true;
    }
    m_loc_subscribed = true;

    // add a new local subscription
    m_writer_udp_mc.AddLocConnection (process_id_, reader_par_);
    m_writer_shm.AddLocConnection    (process_id_, reader_par_);

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug3, m_topic_name + "::CDataWriter::ApplyLocSubscription");
#endif
  }

  void CDataWriter::RemoveLocSubscription(const std::string& process_id_)
  {
    // remove a local subscription
    m_writer_udp_mc.RemLocConnection (process_id_);
    m_writer_shm.RemLocConnection    (process_id_);

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug3, m_topic_name + "::CDataWriter::RemoveLocSubscription");
#endif
  }

  void CDataWriter::ApplyExtSubscription(const std::string& host_name_, const std::string& process_id_, const std::string& reader_par_)
  {
    SetConnected(true);
    {
      std::lock_guard<std::mutex> lock(m_sub_map_sync);
      m_ext_sub_map[host_name_] = true;
    }
    m_ext_subscribed = true;

    // add a new external subscription
    m_writer_udp_mc.AddExtConnection (host_name_, process_id_, reader_par_);
    m_writer_shm.AddExtConnection    (host_name_, process_id_, reader_par_);

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug3, m_topic_name + "::CDataWriter::ApplyExtSubscription");
#endif
  }

  void CDataWriter::RemoveExtSubscription(const std::string& host_name_, const std::string& process_id_)
  {
    // remove external subscription
    m_writer_udp_mc.RemExtConnection (host_name_, process_id_);
    m_writer_shm.RemExtConnection    (host_name_, process_id_);
  }

  void CDataWriter::RefreshRegistration()
  {
    if (!m_created) return;

    // force to register every second to refresh data clock information
    auto curr_time = std::chrono::steady_clock::now();
    auto diff_time = curr_time - m_snd_time;
    if (diff_time > std::chrono::nanoseconds::zero())
    {
      // reset clock and time on first call
      if (m_clock_old == 0)
      {
        m_clock_old = m_clock;
        m_snd_time = curr_time;
      }

      // check for clock difference
      if ((m_clock - m_clock_old) > 0)
      {
        // calculate frequency in mHz
        m_freq = static_cast<long>((1000 * 1000 * (m_clock - m_clock_old)) / std::chrono::duration_cast<std::chrono::milliseconds>(curr_time - m_snd_time).count());
        // reset clock and time
        m_clock_old = m_clock;
        m_snd_time  = curr_time;
      }
      else
      {
        m_freq = 0;
      }
    }

    // register without send
    DoRegister(false);

    // check connection timeouts
    std::shared_ptr<std::list<std::string>> loc_timeouts = std::shared_ptr<std::list<std::string>>(std::make_shared<std::list<std::string>>());
    {
      std::lock_guard<std::mutex> lock(m_sub_map_sync);
      m_loc_sub_map.remove_deprecated(loc_timeouts.get());
      m_ext_sub_map.remove_deprecated();

      m_loc_subscribed = !m_loc_sub_map.empty();
      m_ext_subscribed = !m_ext_sub_map.empty();
    }

    for(auto loc_sub : *loc_timeouts)
    {
      m_writer_shm.RemLocConnection(loc_sub);
    }

    if (!m_loc_subscribed && !m_ext_subscribed)
    {
      SetConnected(false);
    }
  }

  void CDataWriter::RefreshSendCounter()
  {
    // increase write clock
    m_clock++;

    // statistics
    g_process_wclock++;
  }

  std::string CDataWriter::Dump(const std::string& indent_ /* = "" */)
  {
    std::stringstream out;

    out << std::endl;
    out << indent_ << "--------------------------------"           << std::endl;
    out << indent_ << " class CDataWriter  "                       << std::endl;
    out << indent_ << "--------------------------------"           << std::endl;
    out << indent_ << "m_host_name:          " << m_host_name      << std::endl;
    out << indent_ << "m_host_id:            " << m_host_id        << std::endl;
    out << indent_ << "m_topic_name:         " << m_topic_name     << std::endl;
    out << indent_ << "m_topic_id:           " << m_topic_id       << std::endl;
    out << indent_ << "m_topic_type:         " << m_topic_type     << std::endl;
    out << indent_ << "m_topic_desc:         " << m_topic_desc     << std::endl;
    out << indent_ << "m_id:                 " << m_id             << std::endl;
    out << indent_ << "m_clock:              " << m_clock          << std::endl;
    out << indent_ << "m_created:            " << m_created        << std::endl;
    out << indent_ << "m_loc_subscribed:     " << m_loc_subscribed << std::endl;
    out << indent_ << "m_ext_subscribed:     " << m_ext_subscribed << std::endl;
    out << std::endl;

    return(out.str());
  }

  bool CDataWriter::DoRegister(bool force_)
  {
    if (m_topic_name.empty()) return(false);

    // check share modes
    bool share_ttype(m_use_ttype && g_pubgate() && g_pubgate()->TypeShared());
    if (m_share_ttype != -1)
    {
      share_ttype = m_share_ttype == 1;
    }
    bool share_tdesc(m_use_tdesc && g_pubgate() && g_pubgate()->DescriptionShared());
    if (m_share_tdesc != -1)
    {
      share_tdesc = m_share_tdesc == 1;
    }

    // create command parameter
    eCAL::pb::Sample ecal_reg_sample;
    ecal_reg_sample.set_cmd_type(eCAL::pb::bct_reg_publisher);
    auto ecal_reg_sample_mutable_topic = ecal_reg_sample.mutable_topic();
    ecal_reg_sample_mutable_topic->set_hname(m_host_name);
    ecal_reg_sample_mutable_topic->set_hid(m_host_id);
    ecal_reg_sample_mutable_topic->set_tname(m_topic_name);
    ecal_reg_sample_mutable_topic->set_tid(m_topic_id);
    if (share_ttype) ecal_reg_sample_mutable_topic->set_ttype(m_topic_type);
    if (share_tdesc) ecal_reg_sample_mutable_topic->set_tdesc(m_topic_desc);
    ecal_reg_sample_mutable_topic->set_tsize(google::protobuf::int32(m_topic_size));
    // udp multicast layer
    {
      auto tlayer = ecal_reg_sample_mutable_topic->add_tlayer();
      tlayer->set_type(eCAL::pb::tl_ecal_udp_mc);
      tlayer->set_version(1);
      tlayer->set_confirmed(m_use_udp_mc_confirmed);
      tlayer->set_par("");
    }
    // shm layer
    {
      auto tlayer = ecal_reg_sample_mutable_topic->add_tlayer();
      tlayer->set_type(eCAL::pb::tl_ecal_shm);
      tlayer->set_version(1);
      tlayer->set_confirmed(m_use_shm_confirmed);
      tlayer->set_par(m_writer_shm.GetConectionPar());
    }
    // inproc layer
    {
      auto tlayer = ecal_reg_sample_mutable_topic->add_tlayer();
      tlayer->set_type(eCAL::pb::tl_inproc);
      tlayer->set_version(1);
      tlayer->set_confirmed(m_use_inproc_confirmed);
      tlayer->set_par("");
    }
    ecal_reg_sample_mutable_topic->set_pid(m_pid);
    ecal_reg_sample_mutable_topic->set_pname(m_pname);
    ecal_reg_sample_mutable_topic->set_uname(Process::GetUnitName());
    ecal_reg_sample_mutable_topic->set_did(m_id);
    ecal_reg_sample_mutable_topic->set_dclock(m_clock);
    ecal_reg_sample_mutable_topic->set_dfreq(m_freq);

    size_t loc_connections(0);
    size_t ext_connections(0);
    {
      std::lock_guard<std::mutex> lock(m_sub_map_sync);
      loc_connections = m_loc_sub_map.size();
      ext_connections = m_ext_sub_map.size();
    }
    ecal_reg_sample_mutable_topic->set_connections_loc(google::protobuf::int32(loc_connections));
    ecal_reg_sample_mutable_topic->set_connections_ext(google::protobuf::int32(ext_connections));

    // qos HistoryKind
    switch (m_qos.history_kind)
    {
    case QOS::keep_last_history_qos:
      ecal_reg_sample_mutable_topic->mutable_tqos()->set_history(eCAL::pb::QOS::keep_last_history_qos);
      break;
    case QOS::keep_all_history_qos:
      ecal_reg_sample_mutable_topic->mutable_tqos()->set_history(eCAL::pb::QOS::keep_all_history_qos);
      break;
    default:
      break;
    }
    ecal_reg_sample_mutable_topic->mutable_tqos()->set_history_depth(m_qos.history_kind_depth);
    // qos Reliability
    switch (m_qos.reliability)
    {
    case QOS::best_effort_reliability_qos:
      ecal_reg_sample_mutable_topic->mutable_tqos()->set_reliability(eCAL::pb::QOS::best_effort_reliability_qos);
      break;
    case QOS::reliable_reliability_qos:
      ecal_reg_sample_mutable_topic->mutable_tqos()->set_reliability(eCAL::pb::QOS::reliable_reliability_qos);
      break;
    default:
      break;
    }

    // register publisher
    if (g_entity_register()) g_entity_register()->RegisterTopic(m_topic_name, m_topic_id, ecal_reg_sample, force_);

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug4, m_topic_name + "::CDataWriter::DoRegister");
#endif
    return(true);
  }

  void CDataWriter::SetConnected(bool state_)
  {
    if (m_connected == state_) return;
    m_connected = state_;
    if (m_connected)
    {
      auto iter = m_event_callback_map.find(pub_event_connected);
      if (iter != m_event_callback_map.end())
      {
        SPubEventCallbackData data;
        data.type  = pub_event_connected;
        data.time  = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        data.clock = 0;
        (iter->second)(m_topic_name.c_str(), &data);
      }
    }
    else
    {
      auto iter = m_event_callback_map.find(pub_event_disconnected);
      if (iter != m_event_callback_map.end())
      {
        SPubEventCallbackData data;
        data.type  = pub_event_disconnected;
        data.time  = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        data.clock = 0;
        (iter->second)(m_topic_name.c_str(), &data);
      }
    }
  }

  bool CDataWriter::SetUseUdpMC(TLayer::eSendMode mode_)
  {
    m_use_udp_mc = mode_;
    if (!m_created) return true;

    // log send mode
    LogSendMode(m_use_udp_mc, m_topic_name + "::CDataWriter::Create::UDP_MC_SENDMODE::");

    switch (m_use_udp_mc)
    {
    case TLayer::eSendMode::smode_auto:
    case TLayer::eSendMode::smode_on:
      m_writer_udp_mc.Create(m_host_name, m_topic_name, m_topic_id);
#ifndef NDEBUG
      Logging::Log(log_level_debug4, m_topic_name + "::CDataWriter::Create::UDP_MC_WRITER");
#endif
      break;
    case TLayer::eSendMode::smode_none:
    case TLayer::eSendMode::smode_off:
      m_writer_udp_mc.Destroy();
      break;
    }

    return(true);
  }

  bool CDataWriter::SetUseShm(TLayer::eSendMode mode_)
  {
    m_use_shm = mode_;
    if (!m_created) return true;

    // log send mode
    LogSendMode(m_use_shm, m_topic_name + "::CDataWriter::Create::SHM_SENDMODE::");

    switch (m_use_shm)
    {
    case TLayer::eSendMode::smode_auto:
    case TLayer::eSendMode::smode_on:
      m_writer_shm.Create(m_host_name, m_topic_name, m_topic_id);
#ifndef NDEBUG
      Logging::Log(log_level_debug4, m_topic_name + "::CDataWriter::Create::SHM_WRITER");
#endif
      break;
    case TLayer::eSendMode::smode_none:
    case TLayer::eSendMode::smode_off:
      m_writer_shm.Destroy();
      break;
    }

    return(true);
  }

  bool CDataWriter::SetUseInProc(TLayer::eSendMode mode_)
  {
    m_use_inproc = mode_;
    if (!m_created) return true;

    // log send mode
    LogSendMode(m_use_inproc, m_topic_name + "::CDataWriter::Create::INPROC_SENDMODE::");

    switch (m_use_inproc)
    {
    case TLayer::eSendMode::smode_auto:
    case TLayer::eSendMode::smode_on:
      m_writer_inproc.Create(m_host_name, m_topic_name, m_topic_id);
#ifndef NDEBUG
      Logging::Log(log_level_debug4, m_topic_name + "::CDataWriter::Create::INPROC_WRITER");
#endif
      break;
    default:
      m_writer_inproc.Destroy();
      break;
    }

    return(true);
  }

  bool CDataWriter::IsInternalSubscribedOnly()
  {
    std::string process_id = Process::GetProcessIDAsString();
    bool is_internal_only(true);
    std::lock_guard<std::mutex> lock(m_sub_map_sync);
    for (auto sub : m_loc_sub_map)
    {
      if (sub.first != process_id)
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
}
