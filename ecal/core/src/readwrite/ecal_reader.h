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

#pragma once

#include <ecal/ecal.h>
#include <ecal/ecal_callback.h>
#include <ecal/types/topic_information.h>

#ifdef _MSC_VER
#pragma warning(push, 0) // disable proto warnings
#endif
#include <ecal/core/pb/ecal.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "ecal_expmap.h"

#include <condition_variable>
#include <mutex>
#include <atomic>
#include <set>
#include <queue>

#include <string>
#include <unordered_map>

namespace eCAL
{
  class CDataReader
  {
  public:
    CDataReader();
    ~CDataReader();

    static void InitializeLayers();

    bool Create(const std::string& topic_name_, const STopicInformation& topic_info_);
    bool Destroy();

    bool SetQOS(const QOS::SReaderQOS& qos_);

    bool Receive(std::string& buf_, long long* time_ = nullptr, int rcv_timeout_ms_ = 0);

    bool AddReceiveCallback(ReceiveCallbackT callback_);
    bool RemReceiveCallback();

    bool AddEventCallback(eCAL_Subscriber_Event type_, SubEventCallbackT callback_);
    bool RemEventCallback(eCAL_Subscriber_Event type_);

    bool SetTimeout(int timeout_);

    bool SetAttribute(const std::string& attr_name_, const std::string& attr_value_);
    bool ClearAttribute(const std::string& attr_name_);

    void SetID(const std::set<long long>& id_set_);

    void ApplyLocPublication(const std::string& process_id_, const std::string& tid_, const STopicInformation& tinfo_);
    void RemoveLocPublication(const std::string& process_id_, const std::string& tid_);

    void ApplyExtPublication(const std::string& host_name_, const std::string& process_id_, const std::string& tid_, const STopicInformation& tinfo_);
    void RemoveExtPublication(const std::string& host_name_, const std::string& process_id_, const std::string& tid_);

    void ApplyLocLayerParameter(const std::string& process_id_, const std::string& topic_id_, eCAL::pb::eTLayerType type_, const std::string& parameter_);
    void ApplyExtLayerParameter(const std::string& host_name_, eCAL::pb::eTLayerType type_, const std::string& parameter_);

    std::string Dump(const std::string& indent_ = "");

    bool IsCreated() const {return(m_created);}

    size_t GetPublisherCount() const
    {
      const std::lock_guard<std::mutex> lock(m_pub_map_sync);
      return(m_loc_pub_map.size() + m_ext_pub_map.size());
    }

    std::string      GetTopicName()        const {return(m_topic_name);}
    std::string      GetTopicID()          const {return(m_topic_id);}
    STopicInformation GetTopicInformation() const {return(m_topic_info);}

    void RefreshRegistration();
    void CheckReceiveTimeout();

    size_t AddSample(const std::string& tid_, const char* payload_, size_t size_, long long id_, long long clock_, long long time_, size_t hash_, eCAL::pb::eTLayerType layer_);

  protected:
    void SubscribeToLayers();
    void UnsubscribeFromLayers();

    bool Register(bool force_);
    bool Unregister();

    void Connect(const std::string& tid_, const STopicInformation& topic_info_);
    void Disconnect();
    bool CheckMessageClock(const std::string& tid_, long long current_clock_);

    std::string                               m_host_name;
    int                                       m_host_id;
    int                                       m_pid;
    std::string                               m_pname;
    std::string                               m_topic_name;
    std::string                               m_topic_id;
    STopicInformation                          m_topic_info;
    std::map<std::string, std::string>        m_attr;
    std::atomic<size_t>                       m_topic_size;

    QOS::SReaderQOS                           m_qos;

    std::atomic<bool>                         m_connected;
    using ConnectedMapT = Util::CExpMap<std::string, bool>;
    mutable std::mutex                        m_pub_map_sync;
    ConnectedMapT                             m_loc_pub_map;
    ConnectedMapT                             m_ext_pub_map;

    mutable std::mutex                        m_read_buf_mutex;
    std::condition_variable                   m_read_buf_cv;
    bool                                      m_read_buf_received;
    std::string                               m_read_buf;
    long long                                 m_read_time;

    std::mutex                                m_receive_callback_sync;
    ReceiveCallbackT                          m_receive_callback;
    std::atomic<int>                          m_receive_timeout;
    std::atomic<int>                          m_receive_time;

    std::deque<size_t>                        m_sample_hash_queue;

    std::mutex                                m_event_callback_map_sync;
    using EventCallbackMapT = std::map<eCAL_Subscriber_Event, SubEventCallbackT>;
    EventCallbackMapT                         m_event_callback_map;

    std::atomic<long long>                    m_clock;
    long long                                 m_clock_old;
    std::chrono::steady_clock::time_point     m_rec_time;
    long                                      m_freq;

    std::set<long long>                       m_id_set;
    
    using WriterCounterMapT = std::unordered_map<std::string, long long>;
    WriterCounterMapT                         m_writer_counter_map;
    long long                                 m_message_drops;

    std::atomic<bool>                         m_loc_published;
    std::atomic<bool>                         m_ext_published;

    bool                                      m_use_ttype;
    bool                                      m_use_tdesc;

    bool                                      m_use_udp_mc_confirmed;
    bool                                      m_use_shm_confirmed;
    bool                                      m_use_tcp_confirmed;
    bool                                      m_use_inproc_confirmed;

    std::atomic<bool>                         m_created;
  };
}
