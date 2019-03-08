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
#include "ecal_writer_udp_uc.h"
#include "ecal_writer_shm.h"
#include "ecal_writer_lcm.h"
#ifdef ECAL_LAYER_FASTRTPS
#include "ecal_writer_rtps.h"
#endif /* ECAL_LAYER_FASTRTPS */
#include "ecal_writer_inproc.h"

#include <mutex>
#include <string>
#include <atomic>
#include <map>

namespace eCAL
{
  class CDataWriter
  {
    //friend class CDataWriterSHM;

  public:
    CDataWriter();
    ~CDataWriter();

    bool Create(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_);
    bool Destroy();

    bool SetDescription(const std::string& topic_desc_);

    void ShareType(bool state_);
    void ShareDescription(bool state_);

    bool SetQOS(QOS::SWriterQOS& qos_);

    bool SetLayerMode(TLayer::eTransportLayer layer_, TLayer::eSendMode mode_);
    bool SetRefFrequency(double fmin_, double fmax_);
    bool SetMaxBandwidthUDP(long bandwidth_);

    bool AddEventCallback(eCAL_Publisher_Event type_, PubEventCallbackT callback_);
    bool RemEventCallback(eCAL_Publisher_Event type_);

    size_t Send(const void* const buf_, size_t len_, long long time_, long long id_);

    void ApplyLocSubscription(const std::string& process_id_, const std::string& reader_par_);
    void RemoveLocSubscription(const std::string & process_id_);

    void ApplyExtSubscription(const std::string& host_name_, const std::string& process_id_, const std::string& reader_par_);
    void RemoveExtSubscription(const std::string & host_name_, const std::string & process_id_);

    void RefreshRegistration();
    void RefreshSendCounter();

    std::string Dump(const std::string& indent_ = "");

    bool IsCreated() const {return(m_created);}
    bool IsSubscribed() const {return(m_loc_subscribed || m_ext_subscribed);}
    bool IsExtSubscribed() const { return(m_ext_subscribed); }

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
    bool SetUseUdpUC(TLayer::eSendMode mode_);
    bool SetUseShm(TLayer::eSendMode mode_);
    bool SetUseLcm(TLayer::eSendMode mode_);
#ifdef ECAL_LAYER_FASTRTPS
    bool SetUseRtps(TLayer::eSendMode mode_);
#endif /* ECAL_LAYER_FASTRTPS */
    bool SetUseInProc(TLayer::eSendMode mode_);

    bool IsInternalSubscribedOnly();

    void LogSendMode(TLayer::eSendMode smode_, const std::string & base_msg_);

    std::string        m_host_name;
    int                m_pid;
    std::string        m_pname;
    std::string        m_topic_name;
    std::string        m_topic_id;
    std::string        m_topic_type;
    std::string        m_topic_desc;
    size_t             m_topic_size;

    QOS::SWriterQOS    m_qos;

    std::atomic<bool>  m_connected;
    typedef Util::CExpMap<std::string, bool> ConnectedMapT;
    std::mutex         m_sub_map_sync;
    ConnectedMapT      m_loc_sub_map;
    ConnectedMapT      m_ext_sub_map;

    std::mutex         m_event_callback_sync;
    typedef std::map<eCAL_Publisher_Event, PubEventCallbackT> EventCallbackMapT;
    EventCallbackMapT  m_event_callback_map;

    long long          m_id;
    long long          m_clock;
    long long          m_clock_old;
    std::chrono::steady_clock::time_point m_snd_time;
    long               m_freq;
    long               m_freq_min;
    long               m_freq_max;
    long               m_freq_min_err;
    long               m_freq_max_err;

    long               m_bandwidth_max_udp;

    std::atomic<bool>  m_loc_subscribed;
    std::atomic<bool>  m_ext_subscribed;

    TLayer::eSendMode  m_use_udp_mc;
    CDataWriterUdpMC   m_writer_udp_mc;
    bool               m_use_udp_mc_confirmed;

    TLayer::eSendMode  m_use_udp_uc;
    CDataWriterUdpUC   m_writer_udp_uc;
    bool               m_use_udp_uc_confirmed;

    TLayer::eSendMode  m_use_shm;
    CDataWriterSHM     m_writer_shm;
    bool               m_use_shm_confirmed;

    TLayer::eSendMode  m_use_lcm;
    CDataWriterLCM     m_writer_lcm;
    bool               m_use_lcm_confirmed;

#ifdef ECAL_LAYER_FASTRTPS
    TLayer::eSendMode  m_use_rtps;
    CDataWriterRTPS    m_writer_rtps;
    bool               m_use_rtps_confirmed;
#endif /*ECAL_LAYER_FASTRTPS*/

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
