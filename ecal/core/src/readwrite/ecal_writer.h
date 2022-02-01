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

#pragma once

#include <ecal/ecal_callback.h>
#include <ecal/ecal_tlayer.h>

#include "ecal_def.h"
#include "ecal_expmap.h"

#include "ecal_writer_udp_mc.h"

#ifdef ECAL_LAYER_ICEORYX
#include "ecal_writer_iceoryx.h"
#else  /* ECAL_LAYER_ICEORYX */
#include "ecal_writer_shm.h"
#endif /* ECAL_LAYER_ICEORYX */

#include "ecal_writer_tcp.h"
#include "ecal_writer_inproc.h"

#include <mutex>
#include <string>
#include <atomic>
#include <map>

namespace eCAL
{
  class CDataWriter
  {
  public:
    CDataWriter();
    ~CDataWriter();

    bool Create(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_);
    bool Destroy();

    bool SetDescription(const std::string& topic_desc_);

    bool SetAttribute(const std::string& attr_name_, const std::string& attr_value_);
    bool ClearAttribute(const std::string& attr_name_);

    void ShareType(bool state_);
    void ShareDescription(bool state_);

    bool SetQOS(const QOS::SWriterQOS& qos_);

    bool SetLayerMode(TLayer::eTransportLayer layer_, TLayer::eSendMode mode_);
    bool SetMaxBandwidthUDP(long bandwidth_);

    bool ShmSetBufferCount(long buffering_);
    bool ShmEnableZeroCopy(bool state_);

    bool AddEventCallback(eCAL_Publisher_Event type_, PubEventCallbackT callback_);
    bool RemEventCallback(eCAL_Publisher_Event type_);

    bool Write(const void* const buf_, size_t len_, long long time_, long long id_);

    void ApplyLocSubscription(const std::string& process_id_, const std::string& reader_par_);
    void RemoveLocSubscription(const std::string & process_id_);

    void ApplyExtSubscription(const std::string& host_name_, const std::string& process_id_, const std::string& reader_par_);
    void RemoveExtSubscription(const std::string & host_name_, const std::string & process_id_);

    void RefreshRegistration();
    void RefreshSendCounter();

    std::string Dump(const std::string& indent_ = "");

    bool IsCreated() const {return(m_created);}
    bool IsSubscribed() const {return(m_loc_subscribed || m_ext_subscribed);}
    bool IsExtSubscribed() const {return(m_ext_subscribed);}
    size_t GetSubscriberCount() const
    {
      std::lock_guard<std::mutex> lock(m_sub_map_sync);
      return(m_loc_sub_map.size() + m_ext_sub_map.size());
    }

    const std::string& GetTopicName() const {return(m_topic_name);}
    const std::string& GetTopicID() const {return(m_topic_id);}
    const std::string& GetTypeName() const {return(m_topic_type);}
    const std::string& GetDescription() const {return(m_topic_desc);}
    long long GetClock() const {return(m_clock);}
    long GetFrequency() const {return(m_freq);}

  protected:
    bool DoRegister(bool force_);
    void SetConnected(bool state_);

    bool SetUseUdpMC(TLayer::eSendMode mode_);
    bool SetUseShm(TLayer::eSendMode mode_);
    bool SetUseTcp(TLayer::eSendMode mode_);
    bool SetUseInProc(TLayer::eSendMode mode_);

    bool IsInternalSubscribedOnly();

    void LogSendMode(TLayer::eSendMode smode_, const std::string & base_msg_);

    std::string                        m_host_name;
    int                                m_host_id;
    int                                m_pid;
    std::string                        m_pname;
    std::string                        m_topic_name;
    std::string                        m_topic_id;
    std::string                        m_topic_type;
    std::string                        m_topic_desc;
    std::map<std::string, std::string> m_attr;
    size_t                             m_topic_size;

    QOS::SWriterQOS    m_qos;

    size_t             m_buffering_shm;
    bool               m_zero_copy;

    std::atomic<bool>  m_connected;
    typedef Util::CExpMap<std::string, bool> ConnectedMapT;
    mutable std::mutex  m_sub_map_sync;
    ConnectedMapT      m_loc_sub_map;
    ConnectedMapT      m_ext_sub_map;

    std::mutex         m_event_callback_map_sync;
    typedef std::map<eCAL_Publisher_Event, PubEventCallbackT> EventCallbackMapT;
    EventCallbackMapT  m_event_callback_map;

    long long          m_id;
    long long          m_clock;
    long long          m_clock_old;
    std::chrono::steady_clock::time_point m_snd_time;
    long               m_freq;

    long               m_bandwidth_max_udp;

    std::atomic<bool>  m_loc_subscribed;
    std::atomic<bool>  m_ext_subscribed;

    TLayer::eSendMode  m_use_udp_mc;
    CDataWriterUdpMC   m_writer_udp_mc;
    bool               m_use_udp_mc_confirmed;

    TLayer::eSendMode  m_use_shm;
    CDataWriterSHM     m_writer_shm;
    bool               m_use_shm_confirmed;

    TLayer::eSendMode  m_use_tcp;
    CDataWriterTCP     m_writer_tcp;
    bool               m_use_tcp_confirmed;

    TLayer::eSendMode  m_use_inproc;
    CDataWriterInProc  m_writer_inproc;
    bool               m_use_inproc_confirmed;

    bool               m_use_ttype;
    bool               m_use_tdesc;
    int                m_share_ttype;
    int                m_share_tdesc;
    bool               m_created;
  };
}
