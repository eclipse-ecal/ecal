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
 * @brief  common eCAL data reader
**/

#include <ecal/ecal.h>
#include <ecal/ecal_config.h>

#include "ecal_def.h"
#include "ecal_registration_provider.h"
#include "ecal_descgate.h"
#include "ecal_reader.h"
#include "ecal_process.h"

#include "readwrite/ecal_reader_udp_mc.h"

#ifdef ECAL_LAYER_ICEORYX
#include "readwrite/ecal_reader_iceoryx.h"
#else  /* ECAL_LAYER_ICEORYX */
#include "readwrite/ecal_reader_shm.h"
#endif /* ECAL_LAYER_ICEORYX */

#include "readwrite/ecal_reader_tcp.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>

namespace eCAL
{
  ////////////////////////////////////////
  // CDataReader
  ////////////////////////////////////////
  CDataReader::CDataReader() :
                 m_host_name(Process::GetHostName()),
                 m_host_id(Process::internal::GetHostID()),
                 m_pid(Process::GetProcessID()),
                 m_pname(Process::GetProcessName()),
                 m_topic_name(""),
                 m_topic_id(""),
                 m_topic_type(""),
                 m_topic_size(0),
                 m_connected(false),
                 m_read_buf_received(false),
                 m_read_time(0),
                 m_receive_timeout(0),
                 m_receive_time(0),
                 m_clock(0),
                 m_clock_old(0),
                 m_rec_time(),
                 m_freq(0),
                 m_message_drops(0),
                 m_loc_published(false),
                 m_ext_published(false),
                 m_use_ttype(true),
                 m_use_tdesc(true),
                 m_use_udp_mc_confirmed(false),
                 m_use_shm_confirmed(false),
                 m_use_tcp_confirmed(false),
                 m_use_inproc_confirmed(false),
                 m_created(false)
  {
  }

  CDataReader::~CDataReader()
  {
    Destroy();
  }

  bool CDataReader::Create(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_)
  {
    if(m_created) return(false);

    // set defaults
    m_topic_name    = topic_name_;
    m_topic_id.clear();
    m_topic_type    = topic_type_;
    m_topic_desc    = topic_desc_;
    m_clock         = 0;
    m_clock_old     = 0;
    m_message_drops = 0;
    m_rec_time      = std::chrono::steady_clock::time_point();
    m_created       = false;
#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug1, m_topic_name + "::CDataReader::Create");
#endif
    // build topic id
    std::stringstream counter;
    counter << std::chrono::steady_clock::now().time_since_epoch().count();
    m_topic_id = counter.str();

    // set registration expiration
    std::chrono::milliseconds registration_timeout(Config::GetRegistrationTimeoutMs());
    m_loc_pub_map.set_expiration(registration_timeout);
    m_ext_pub_map.set_expiration(registration_timeout);

    // set sample hash map expiration
    m_sample_hash.set_expiration(std::chrono::milliseconds(500));

    // allow to share topic type
    m_use_ttype = Config::IsTopicTypeSharingEnabled();

    // allow to share topic description
    m_use_tdesc = Config::IsTopicDescriptionSharingEnabled();

    // start transport layers
    SubscribeToLayers();

    // register
    DoRegister(false);

    // mark as created
    m_created = true;

    return(true);
  }

  bool CDataReader::Destroy()
  {
    if (!m_created) return(false);

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug1, m_topic_name + "::CDataReader::Destroy");
#endif

    // stop transport layers
    UnsubscribeFromLayers();

    // reset receive callback
    {
      std::lock_guard<std::mutex> lock(m_receive_callback_sync);
      m_receive_callback = nullptr;
    }

    // reset event callback map
    {
      std::lock_guard<std::mutex> lock(m_event_callback_map_sync);
      m_event_callback_map.clear();
    }

    // reset defaults
    m_created                 = false;
    m_clock                   = 0;
    m_clock_old               = 0;
    m_freq                    = 0;
    m_message_drops           = 0;
    m_rec_time                = std::chrono::steady_clock::time_point();

    m_use_udp_mc_confirmed    = false;
    m_use_shm_confirmed       = false;
    m_use_tcp_confirmed       = false;
    m_use_inproc_confirmed    = false;

    return(true);
  }
    
  void CDataReader::InitializeLayers()
  {
    // initialize udp multicast layer
    if (Config::IsUdpMulticastRecEnabled())
    {
      CUDPReaderLayer::Get()->Initialize();
    }

    // initialize tcp layer
    if (Config::IsTcpRecEnabled())
    {
      CTCPReaderLayer::Get()->Initialize();
    }
  }

  void CDataReader::SubscribeToLayers()
  {
    // subscribe topic to udp multicast layer
    if (Config::IsUdpMulticastRecEnabled())
    {
      CUDPReaderLayer::Get()->AddSubscription(m_host_name, m_topic_name, m_topic_id, m_qos);
    }

    // subscribe topic to tcp layer
    if (Config::IsTcpRecEnabled())
    {
      CTCPReaderLayer::Get()->AddSubscription(m_host_name, m_topic_name, m_topic_id, m_qos);
    }
  }
  
  void CDataReader::UnsubscribeFromLayers()
  {
    // unsubscribe topic from udp multicast layer
    if (Config::IsUdpMulticastRecEnabled())
    {
      CUDPReaderLayer::Get()->RemSubscription(m_host_name, m_topic_name, m_topic_id);
    }

    // unsubscribe topic from tcp multicast layer
    if (Config::IsTcpRecEnabled())
    {
      CTCPReaderLayer::Get()->RemSubscription(m_host_name, m_topic_name, m_topic_id);
    }
  }

  bool CDataReader::DoRegister(const bool force_)
  {
    if(!m_created)           return(false);
    if(m_topic_name.empty()) return(false);

    // create command parameter
    eCAL::pb::Sample ecal_reg_sample;
    ecal_reg_sample.set_cmd_type(eCAL::pb::bct_reg_subscriber);
    auto ecal_reg_sample_mutable_topic = ecal_reg_sample.mutable_topic();
    ecal_reg_sample_mutable_topic->set_hname(m_host_name);
    ecal_reg_sample_mutable_topic->set_hid(m_host_id);
    ecal_reg_sample_mutable_topic->set_tname(m_topic_name);
    ecal_reg_sample_mutable_topic->set_tid(m_topic_id);
    if (m_use_ttype) ecal_reg_sample_mutable_topic->set_ttype(m_topic_type);
    if (m_use_tdesc) ecal_reg_sample_mutable_topic->set_tdesc(m_topic_desc);
    *ecal_reg_sample_mutable_topic->mutable_attr() = google::protobuf::Map<std::string, std::string> { m_attr.begin(), m_attr.end() };
    ecal_reg_sample_mutable_topic->set_tsize(google::protobuf::int32(m_topic_size));
    // udp multicast layer
    {
      auto tlayer = ecal_reg_sample_mutable_topic->add_tlayer();
      tlayer->set_type(eCAL::pb::tl_ecal_udp_mc);
      tlayer->set_version(1);
      tlayer->set_confirmed(m_use_udp_mc_confirmed);
    }
    // shm layer
    {
      auto tlayer = ecal_reg_sample_mutable_topic->add_tlayer();
      tlayer->set_type(eCAL::pb::tl_ecal_shm);
      tlayer->set_version(1);
      tlayer->set_confirmed(m_use_shm_confirmed);
    }
    // tcp layer
    {
      auto tlayer = ecal_reg_sample_mutable_topic->add_tlayer();
      tlayer->set_type(eCAL::pb::tl_ecal_tcp);
      tlayer->set_version(1);
      tlayer->set_confirmed(m_use_tcp_confirmed);
    }
    // inproc layer
    {
      auto tlayer = ecal_reg_sample_mutable_topic->add_tlayer();
      tlayer->set_type(eCAL::pb::tl_inproc);
      tlayer->set_version(1);
      tlayer->set_confirmed(m_use_inproc_confirmed);
    }
    ecal_reg_sample_mutable_topic->set_pid(m_pid);
    ecal_reg_sample_mutable_topic->set_pname(m_pname);
    ecal_reg_sample_mutable_topic->set_uname(Process::GetUnitName());
    ecal_reg_sample_mutable_topic->set_dclock(m_clock);
    ecal_reg_sample_mutable_topic->set_dfreq(m_freq);
    ecal_reg_sample_mutable_topic->set_message_drops(google::protobuf::int32(m_message_drops));

    size_t loc_connections(0);
    size_t ext_connections(0);
    {
      std::lock_guard<std::mutex> lock(m_pub_map_sync);
      loc_connections = m_loc_pub_map.size();
      ext_connections = m_ext_pub_map.size();
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
    }

    // register subscriber
    if(g_registration_provider()) g_registration_provider()->RegisterTopic(m_topic_name, m_topic_id, ecal_reg_sample, force_);
#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug4, m_topic_name + "::CDataReader::DoRegister");
#endif
    return(true);
  }

  bool CDataReader::SetQOS(const QOS::SReaderQOS& qos_)
  {
    m_qos = qos_;
    return (!m_created);
  }

  bool CDataReader::SetAttribute(const std::string& attr_name_, const std::string& attr_value_)
  {
    auto current_val = m_attr.find(attr_name_);

    bool force = current_val == m_attr.end() || current_val->second != attr_value_;
    m_attr[attr_name_] = attr_value_;

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug2, m_topic_name + "::CDataReader::SetAttribute");
#endif

    // register it
    DoRegister(force);

    return(true);
  }

  bool CDataReader::ClearAttribute(const std::string& attr_name_)
  {
    auto force = m_attr.find(attr_name_) != m_attr.end();

    m_attr.erase(attr_name_);

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug2, m_topic_name + "::CDataReader::ClearAttribute");
#endif

    // register it
    DoRegister(force);

    return(true);
  }

  bool CDataReader::Receive(std::string& buf_, long long* time_ /* = nullptr */, int rcv_timeout_ms_ /* = 0 */)
  {
    if (!m_created) return(false);

    std::unique_lock<std::mutex> read_buffer_lock(m_read_buf_mutex);

    // No need to wait (for whatever time) if something has been received)
    if (!m_read_buf_received)
    {
      if (rcv_timeout_ms_ < 0)
      {
        m_read_buf_cv.wait(read_buffer_lock, [this]() { return this->m_read_buf_received; });
      }
      else if (rcv_timeout_ms_ > 0)
      {
        m_read_buf_cv.wait_for(read_buffer_lock, std::chrono::milliseconds(rcv_timeout_ms_), [this]() { return this->m_read_buf_received; });
      }
    }

    // did we receive new samples ?
    if (m_read_buf_received)
    {
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug3, m_topic_name + "::CDataReader::Receive");
#endif
      // copy content to target string
      buf_.clear();
      buf_.swap(m_read_buf);
      m_read_buf_received = false;

      // apply time
      if(time_) *time_ = m_read_time;

      // return success
      return(true);
    }

    return(false);
  }

  size_t CDataReader::AddSample(const std::string& tid_, const char* payload_, size_t size_, long long id_, long long clock_, long long time_, size_t hash_, eCAL::pb::eTLayerType layer_)
  {
    // ensure thread safety
    std::lock_guard<std::mutex> lock(m_receive_callback_sync);
    if (!m_created) return(0);

    // store receive layer
    m_use_udp_mc_confirmed |= layer_ == eCAL::pb::tl_ecal_udp_mc;
    m_use_shm_confirmed    |= layer_ == eCAL::pb::tl_ecal_shm;
    m_use_tcp_confirmed    |= layer_ == eCAL::pb::tl_ecal_tcp;
    m_use_inproc_confirmed |= layer_ == eCAL::pb::tl_inproc;

    // use hash to discard multiple receives of the same payload
    //   first we remove outdated hashes
    m_sample_hash.remove_deprecated();
    //   if this hash is still in the map
    //   we received it recently (on another transport layer ?)
    //   so we return and do not process this sample again
    if (m_sample_hash.find(hash_) != m_sample_hash.end())
    {
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug3, m_topic_name + "::CDataReader::AddSample discard sample because of multiple receive");
#endif
      return(size_);
    }
    //   this is a new sample -> store it's hash
    m_sample_hash[hash_] = 0;

    // check id
    if (!m_id_set.empty())
    {
      if (m_id_set.find(id_) == m_id_set.end()) return(0);
    }

    // check the current message clock
    // if the function returns false we detected
    //  - a dropped message
    //  - an out of order message
    //  - a multiple sent message
    if (!CheckMessageClock(tid_, clock_))
    {
      // we will not process that message
      return(0);
    }

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug3, m_topic_name + "::CDataReader::AddSample");
#endif

    // increase read clock
    m_clock++;

    // reset timeout
    m_receive_time = 0;

    // store size
    m_topic_size = size_;

    // execute callback
    bool processed = false;
    {
      // call user receive callback function
      if(m_receive_callback)
      {
#ifndef NDEBUG
        // log it
        Logging::Log(log_level_debug3, m_topic_name + "::CDataReader::AddSample::ReceiveCallback");
#endif
        // prepare data struct
        SReceiveCallbackData cb_data;
        cb_data.buf   = const_cast<char*>(payload_);
        cb_data.size  = long(size_);
        cb_data.id    = id_;
        cb_data.time  = time_;
        cb_data.clock = clock_;
        // execute it
        (m_receive_callback)(m_topic_name.c_str(), &cb_data);
        processed = true;
      }
    }

    // if not consumed by user receive call
    if(!processed)
    {
      // push sample into read buffer
      std::lock_guard<std::mutex> read_buffer_lock(m_read_buf_mutex);
      m_read_buf.clear();
      m_read_buf.assign(payload_, payload_ + size_);
      m_read_time = time_;
      m_read_buf_received = true;

      // inform receive
      m_read_buf_cv.notify_one();
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug3, m_topic_name + "::CDataReader::AddSample::Receive::Buffered");
#endif
    }

    return(size_);
  }

  bool CDataReader::AddReceiveCallback(ReceiveCallbackT callback_)
  {
    if (!m_created) return(false);

    // store receive callback
    {
      std::lock_guard<std::mutex> lock(m_receive_callback_sync);
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug2, m_topic_name + "::CDataReader::AddReceiveCallback");
#endif
      m_receive_callback = callback_;
    }

    return(true);
  }

  bool CDataReader::RemReceiveCallback()
  {
    if (!m_created) return(false);

    // reset receive callback
    {
      std::lock_guard<std::mutex> lock(m_receive_callback_sync);
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug2, m_topic_name + "::CDataReader::RemReceiveCallback");
#endif
      m_receive_callback = nullptr;
    }

    return(true);
  }

  bool CDataReader::AddEventCallback(eCAL_Subscriber_Event type_, SubEventCallbackT callback_)
  {
    if (!m_created) return(false);

    // store event callback
    {
      std::lock_guard<std::mutex> lock(m_event_callback_map_sync);
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug2, m_topic_name + "::CDataReader::AddEventCallback");
#endif
      m_event_callback_map[type_] = callback_;
    }

    return(true);
  }

  bool CDataReader::RemEventCallback(eCAL_Subscriber_Event type_)
  {
    if (!m_created) return(false);

    // reset event callback
    {
      std::lock_guard<std::mutex> lock(m_event_callback_map_sync);
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug2, m_topic_name + "::CDataReader::RemEventCallback");
#endif
      m_event_callback_map[type_] = nullptr;
    }

    return(true);
  }

  bool CDataReader::SetTimeout(int timeout_)
  {
    m_receive_timeout = timeout_;
    return(false);
  }

  void CDataReader::SetID(const std::set<long long>& id_set_)
  {
    m_id_set = id_set_;
  }

  void CDataReader::ApplyLocPublication(const std::string& process_id_, const std::string& tid_, const std::string& ttype_, const std::string& tdesc_)
  {
    Connect(tid_, ttype_, tdesc_);
    {
      std::lock_guard<std::mutex> lock(m_pub_map_sync);
      m_loc_pub_map[process_id_] = true;
    }
    m_loc_published = true;
  }

  void CDataReader::ApplyExtPublication(const std::string& host_name_, const std::string& tid_, const std::string& ttype_, const std::string& tdesc_)
  {
    Connect(tid_, ttype_, tdesc_);
    {
      std::lock_guard<std::mutex> lock(m_pub_map_sync);
      m_ext_pub_map[host_name_] = true;
    }
    m_ext_published = true;
  }

  void CDataReader::ApplyLocLayerParameter(const std::string& process_id_, const std::string& topic_id_, eCAL::pb::eTLayerType type_, const std::string& parameter_)
  {
    // process only for shm and tcp layer
    switch (type_)
    {
    case eCAL::pb::tl_ecal_shm:
      break;
    case eCAL::pb::tl_ecal_tcp:
      break;
    default:
      return;
    }

    SReaderLayerPar par;
    par.host_name  = m_host_name;
    par.process_id = process_id_;
    par.topic_name = m_topic_name;
    par.topic_id   = topic_id_;
    par.parameter  = parameter_;

    switch (type_)
    {
    case eCAL::pb::tl_ecal_shm:
      CSHMReaderLayer::Get()->SetConnectionParameter(par);
      break;
    case eCAL::pb::tl_ecal_tcp:
      CTCPReaderLayer::Get()->SetConnectionParameter(par);
      break;
    default:
      break;
    }
  }

  void CDataReader::ApplyExtLayerParameter(const std::string& host_name_, eCAL::pb::eTLayerType type_, const std::string& parameter_)
  {
    // process only for tcp layer
    switch (type_)
    {
    case eCAL::pb::tl_ecal_tcp:
      break;
    default:
      return;
    }

    SReaderLayerPar par;
    par.host_name  = host_name_;
    par.topic_name = m_topic_name;
    par.topic_id   = m_topic_id;
    par.parameter  = parameter_;

    switch (type_)
    {
    case eCAL::pb::tl_ecal_tcp:
      CTCPReaderLayer::Get()->SetConnectionParameter(par);
      break;
    default:
      break;
    }
  }

  void CDataReader::Connect(const std::string& tid_, const std::string& ttype_, const std::string& tdesc_)
  {
    SSubEventCallbackData data;
    data.time  = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    data.clock = 0;

    if (!m_connected)
    {
      m_connected = true;

      // fire sub_event_connected
      {
        auto iter = m_event_callback_map.find(sub_event_connected);
        if (iter != m_event_callback_map.end())
        {
          data.type = sub_event_connected;
          (iter->second)(m_topic_name.c_str(), &data);
        }
      }
    }

    // fire sub_event_update_connection
    auto iter = m_event_callback_map.find(sub_event_update_connection);
    if (iter != m_event_callback_map.end())
    {
      data.type  = sub_event_update_connection;
      data.tid   = tid_;
      data.ttype = ttype_;
      data.tdesc = tdesc_;
      (iter->second)(m_topic_name.c_str(), &data);
    }
  }

  void CDataReader::Disconnect()
  {
    if (m_connected)
    {
      m_connected = false;

      // fire sub_event_disconnected
      auto iter = m_event_callback_map.find(sub_event_disconnected);
      if (iter != m_event_callback_map.end())
      {
        SSubEventCallbackData data;
        data.type  = sub_event_disconnected;
        data.time  = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        data.clock = 0;
        (iter->second)(m_topic_name.c_str(), &data);
      }
    }
  }

  bool CDataReader::CheckMessageClock(const std::string& tid_, long long current_clock_)
  {
    auto iter = m_writer_counter_map.find(tid_);
    
    // initial entry
    if (iter == m_writer_counter_map.end())
    {
      m_writer_counter_map[tid_] = current_clock_;
      return true;
    }
    // clock entry exists
    else
    {
      // calculate difference
      long long last_clock = iter->second;
      long long clock_difference = current_clock_ - last_clock;

      // this is perfect, the next message arrived
      if (clock_difference == 1)
      {
        // update the internal clock counter
        iter->second = current_clock_;

        // process it
        return true;
      }

      // that should never happen, maybe there is a publisher
      // sending parallel on multiple layers ?
      // we ignore this message duplicate
      if (clock_difference == 0)
      {
        // do not update the internal clock counter

        // do not process it
        return false;
      }

      // that means we miss at least one message
      // -> we have a "message drop"
      if (clock_difference > 1)
      {
#if 0
        // we log this
        std::string msg = std::to_string(counter_ - counter_last) + " Messages lost ! ";
        msg += "(Unit: \'";
        msg += Process::GetUnitName();
        msg += "@";
        msg += Process::GetHostName();
        msg += "\' | Subscriber: \'";
        msg += m_topic_name;
        msg += "\')";
        Logging::Log(log_level_warning, msg);
#endif
        // we fire the message drop event
        auto citer = m_event_callback_map.find(sub_event_dropped);
        if (citer != m_event_callback_map.end())
        {
          SSubEventCallbackData data;
          data.type = sub_event_dropped;
          data.time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
          data.clock = current_clock_;
          (citer->second)(m_topic_name.c_str(), &data);
        }
        // increase the drop counter
        m_message_drops += clock_difference;

        // update the internal clock counter
        iter->second = current_clock_;

        // process it
        return true;
      }

      // a negative clock difference may happen if a publisher
      // is using a shm ringbuffer and messages arrive in the wrong order
      if (clock_difference < 0)
      {
        // -----------------------------------
        // drop messages in the wrong order
        // -----------------------------------
        if (eCAL::Config::Experimental::GetDropOutOfOrderMessages())
        {
          // do not update the internal clock counter

          // there is no need to fire the drop event, because
          // this event has been fired with the message before

          // do not process it
          return false;
        }
        // -----------------------------------
        // process messages in the wrong order
        // -----------------------------------
        else
        {
          // do not update the internal clock counter

          // but we log this
          std::string msg = "Subscriber: \'";
          msg += m_topic_name;
          msg += "\'";
          msg += " received a message in the wrong order";
          Logging::Log(log_level_warning, msg);

          // process it
          return true;
        }
      }
    }

    // should never be reached
    return false;
  }
    
  void CDataReader::RefreshRegistration()
  {
    if(!m_created) return;

    // ensure that registration is not called within zero nanoseconds
    // normally it will be called from registration logic every second
    auto curr_time = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(curr_time - m_rec_time) > std::chrono::milliseconds(0))
    {
      // reset clock and time on first call
      if (m_clock_old == 0)
      {
        m_clock_old = m_clock;
        m_rec_time  = curr_time;
      }

      // check for clock difference
      else if ((m_clock - m_clock_old) > 0)
      {
        // calculate frequency in mHz
        m_freq = static_cast<long>((1000 * 1000 * (m_clock - m_clock_old)) / std::chrono::duration_cast<std::chrono::milliseconds>(curr_time - m_rec_time).count());
        // reset clock and time
        m_clock_old = m_clock;
        m_rec_time  = curr_time;
      }
      else
      {
        m_freq = 0;
      }
    }

    // register without send
    DoRegister(false);

    // check connection timeouts
    std::shared_ptr<std::list<std::string>> loc_timeouts = std::make_shared<std::list<std::string>>();
    {
      std::lock_guard<std::mutex> lock(m_pub_map_sync);
      m_loc_pub_map.remove_deprecated(loc_timeouts.get());
      m_ext_pub_map.remove_deprecated();

      m_loc_published = !m_loc_pub_map.empty();
      m_ext_published = !m_ext_pub_map.empty();
    }

    if (!m_loc_published && !m_ext_published)
    {
      Disconnect();
    }
  }

  void CDataReader::CheckReceiveTimeout()
  {
    // check receive timeout
    if(m_receive_timeout > 0)
    {
      m_receive_time += CMN_DATAREADER_TIMEOUT_DTIME;
      if(m_receive_time > m_receive_timeout)
      {
        std::lock_guard<std::mutex> lock(m_event_callback_map_sync);
        auto iter = m_event_callback_map.find(sub_event_timeout);
        if(iter != m_event_callback_map.end())
        {
          SSubEventCallbackData data;
          data.type  = sub_event_timeout;
          data.time  = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
          data.clock = 0;
          (iter->second)(m_topic_name.c_str(), &data);
          m_receive_time = 0;
        }
      }
    }
  }

  const std::string CDataReader::GetDescription() const
  {
    std::string topic_desc;
    if(g_descgate() && g_descgate()->GetTopicDescription(m_topic_name, topic_desc))
    {
      return(topic_desc);
    }
    return("");
  }

  std::string CDataReader::Dump(const std::string& indent_ /* = "" */)
  {
    std::stringstream out;

    out << std::endl;
    out << indent_ << "--------------------------------"                   << std::endl;
    out << indent_ << " class CDataReader "                                << std::endl;
    out << indent_ << "--------------------------------"                   << std::endl;
    out << indent_ << "m_host_name:            " << m_host_name            << std::endl;
    out << indent_ << "m_host_id:              " << m_host_id              << std::endl;
    out << indent_ << "m_topic_name:           " << m_topic_name           << std::endl;
    out << indent_ << "m_topic_id:             " << m_topic_id             << std::endl;
    out << indent_ << "m_topic_type:           " << m_topic_type           << std::endl;
    out << indent_ << "m_topic_desc:           " << m_topic_desc           << std::endl;
    out << indent_ << "m_topic_size:           " << m_topic_size           << std::endl;
    out << indent_ << "m_read_buf.size():      " << m_read_buf.size()      << std::endl;
    out << indent_ << "m_read_time:            " << m_read_time            << std::endl;
    out << indent_ << "m_clock:                " << m_clock                << std::endl;
    out << indent_ << "m_rec_time:             " << std::chrono::duration_cast<std::chrono::milliseconds>(m_rec_time.time_since_epoch()).count() << std::endl;
    out << indent_ << "m_freq:                 " << m_freq                 << std::endl;
    out << indent_ << "m_created:              " << m_created              << std::endl;
    out << std::endl;

    return(out.str());
  }
}
