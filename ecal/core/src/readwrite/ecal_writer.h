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
#include <ecal/ecal_payload_writer.h>
#include <ecal/ecal_tlayer.h>
#include <ecal/ecal_types.h>

#include "ecal_def.h"
#include "ecal_expmap.h"

#include "ecal_writer_udp_mc.h"
#include "ecal_writer_shm.h"
#include "ecal_writer_tcp.h"
#include "ecal_writer_inproc.h"

#include <mutex>
#include <string>
#include <atomic>
#include <map>
#include <vector>

namespace eCAL
{
  class CDataWriter
  {
  public:
    struct SExternalSubscriptionInfo
    {
      std::string host_name;
      std::string process_id;
      std::string topic_id;

      friend bool operator<(const SExternalSubscriptionInfo& l, const SExternalSubscriptionInfo& r)
      {
        return std::tie(l.host_name, l.process_id, l.topic_id)
          < std::tie(r.host_name, r.process_id, r.topic_id);
      }
    };

    struct SLocalSubscriptionInfo
    {
      std::string process_id;
      std::string topic_id;

      friend bool operator<(const SLocalSubscriptionInfo& l, const SLocalSubscriptionInfo& r)
      {
        return std::tie(l.process_id, l.topic_id)
          < std::tie(r.process_id, r.topic_id);
      }
    };

    CDataWriter();
    ~CDataWriter();

    bool Create(const std::string& topic_name_, const SDataTypeInformation& topic_info_);
    bool Destroy();

    bool SetDataTypeInformation(const SDataTypeInformation& topic_info_);

    bool SetAttribute(const std::string& attr_name_, const std::string& attr_value_);
    bool ClearAttribute(const std::string& attr_name_);

    void ShareType(bool state_);
    void ShareDescription(bool state_);

    bool SetQOS(const QOS::SWriterQOS& qos_);

    bool SetLayerMode(TLayer::eTransportLayer layer_, TLayer::eSendMode mode_);
    bool SetMaxBandwidthUDP(long bandwidth_);

    bool ShmSetBufferCount(size_t buffering_);
    bool ShmEnableZeroCopy(bool state_);

    bool ShmSetAcknowledgeTimeout(long long acknowledge_timeout_ms_);
    long long ShmGetAcknowledgeTimeout() const;

    bool AddEventCallback(eCAL_Publisher_Event type_, PubEventCallbackT callback_);
    bool RemEventCallback(eCAL_Publisher_Event type_);

    size_t Write(CPayloadWriter& payload_, long long time_, long long id_);

    void ApplyLocSubscription(const SLocalSubscriptionInfo& local_info_, const SDataTypeInformation& tinfo_, const std::string& reader_par_);
    void RemoveLocSubscription(const SLocalSubscriptionInfo& local_info_);

    void ApplyExtSubscription(const SExternalSubscriptionInfo& external_info_, const SDataTypeInformation& tinfo_, const std::string& reader_par_);
    void RemoveExtSubscription(const SExternalSubscriptionInfo& external_info_);

    void RefreshRegistration();
    void RefreshSendCounter();

    std::string Dump(const std::string& indent_ = "");

    bool IsCreated() const {return(m_created);}
    bool IsSubscribed() const {return(m_loc_subscribed || m_ext_subscribed);}
    bool IsExtSubscribed() const {return(m_ext_subscribed);}
    size_t GetSubscriberCount() const
    {
      std::lock_guard<std::mutex> const lock(m_sub_map_sync);
      return(m_loc_sub_map.size() + m_ext_sub_map.size());
    }

    const std::string& GetTopicName() const {return(m_topic_name);}
    const SDataTypeInformation& GetDataTypeInformation() const { return m_topic_info; }

  protected:
    bool Register(bool force_);
    bool Unregister();

    void Connect(const std::string& tid_, const SDataTypeInformation& tinfo_);
    void Disconnect();

    void SetUseUdpMC(TLayer::eSendMode mode_);
    void SetUseShm(TLayer::eSendMode mode_);
    void SetUseTcp(TLayer::eSendMode mode_);
    void SetUseInProc(TLayer::eSendMode mode_);

    bool CheckWriterModes();
    size_t PrepareWrite(long long id_, size_t len_);
    bool IsInternalSubscribedOnly();
    void LogSendMode(TLayer::eSendMode smode_, const std::string & base_msg_);

    std::string                        m_host_name;
    std::string                        m_host_group_name;
    int                                m_host_id;
    int                                m_pid;
    std::string                        m_pname;
    std::string                        m_topic_name;
    std::string                        m_topic_id;
    SDataTypeInformation               m_topic_info;
    std::map<std::string, std::string> m_attr;
    size_t                             m_topic_size;

    QOS::SWriterQOS    m_qos;

    size_t             m_buffering_shm;
    bool               m_zero_copy;
    long long          m_acknowledge_timeout_ms;

    std::vector<char>  m_payload_buffer;

    std::atomic<bool>  m_connected;

    using LocalConnectedMapT = Util::CExpMap<SLocalSubscriptionInfo, bool>;
    using ExternalConnectedMapT = Util::CExpMap<SExternalSubscriptionInfo, bool>;
    mutable std::mutex    m_sub_map_sync;
    LocalConnectedMapT    m_loc_sub_map;
    ExternalConnectedMapT m_ext_sub_map;

    std::mutex         m_event_callback_map_sync;
    using EventCallbackMapT = std::map<eCAL_Publisher_Event, PubEventCallbackT>;
    EventCallbackMapT  m_event_callback_map;

    long long          m_id;
    long long          m_clock;
    long long          m_clock_old;
    std::chrono::steady_clock::time_point m_snd_time;
    long               m_freq;

    long               m_bandwidth_max_udp;

    std::atomic<bool>  m_loc_subscribed;
    std::atomic<bool>  m_ext_subscribed;

    struct SWriter
    {
      struct SWriterMode
      {
        TLayer::eSendMode requested = TLayer::smode_off;
        bool              activated = false;
        bool              confirmed = false;
      };

      SWriterMode        udp_mc_mode;
      CDataWriterUdpMC   udp_mc;

      SWriterMode        shm_mode;
      CDataWriterSHM     shm;

      SWriterMode        tcp_mode;
      CDataWriterTCP     tcp;

      SWriterMode        inproc_mode;
      CDataWriterInProc  inproc;
    };
    SWriter            m_writer;

    bool               m_use_ttype;
    bool               m_use_tdesc;
    int                m_share_ttype;
    int                m_share_tdesc;
    bool               m_created;
  };
}
