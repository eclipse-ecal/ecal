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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include "ecal/pb/ecal.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "ecal_expmap.h"

#include <mutex>
#include <atomic>
#include <set>

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

    bool Create(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_);
    bool Destroy();

    bool SetQOS(const QOS::SReaderQOS& qos_);

    bool Receive(std::string& buf_, long long* time_ = nullptr, int rcv_timeout_ = 0);

    bool AddReceiveCallback(ReceiveCallbackT callback_);
    bool RemReceiveCallback();

    bool AddEventCallback(eCAL_Subscriber_Event type_, SubEventCallbackT callback_);
    bool RemEventCallback(eCAL_Subscriber_Event type_);

    bool SetTimeout(int timeout_);

    bool SetAttribute(const std::string& attr_name_, const std::string& attr_value_);
    bool ClearAttribute(const std::string& attr_name_);

    void SetID(const std::set<long long>& id_set_);

    void ApplyLocPublication(const std::string& process_id_);
    void ApplyExtPublication(const std::string& host_name_);

    void ApplyLocLayerParameter(const std::string& process_id_, eCAL::pb::eTLayerType type_, const std::string& parameter_);
    void ApplyExtLayerParameter(const std::string& host_name_,  eCAL::pb::eTLayerType type_, const std::string& parameter_);

    std::string Dump(const std::string& indent_ = "");

    bool IsCreated() const {return(m_created);}

    size_t GetPublisherCount() const
    {
      std::lock_guard<std::mutex> lock(m_pub_map_sync);
      return(m_loc_pub_map.size() + m_ext_pub_map.size());
    }

    const std::string GetTopicName()                              const {return(m_topic_name);}
    const std::string GetTopicID()                                const {return(m_topic_id);}
    const std::string GetTypeName()                               const {return(m_topic_type);}
    const std::string GetDescription()                            const;

    void RefreshRegistration();
    void CheckReceiveTimeout();

    size_t AddSample(const std::string& tid_, const char* payload_, size_t size_, long long id_, long long clock_, long long time_, size_t hash_, eCAL::pb::eTLayerType layer_);

  protected:
    void StartDataLayers();
    void StopDataLayers();

    bool DoRegister(const bool force_);
    void SetConnected(bool state_);
    void CheckCounter(const std::string& tid_, long long counter_);

    std::string                               m_host_name;
    int                                       m_host_id;
    int                                       m_pid;
    std::string                               m_pname;
    std::string                               m_topic_name;
    std::string                               m_topic_id;
    std::string                               m_topic_type;
    std::string                               m_topic_desc;
    std::map<std::string, std::string>        m_attr;
    std::atomic<size_t>                       m_topic_size;

    QOS::SReaderQOS                           m_qos;

    std::atomic<bool>                         m_connected;
    typedef Util::CExpMap<std::string, bool> ConnectedMapT;
    mutable std::mutex                        m_pub_map_sync;
    ConnectedMapT                             m_loc_pub_map;
    ConnectedMapT                             m_ext_pub_map;

    EventHandleT                              m_receive_event;

    std::mutex                                m_read_buf_sync;
    std::string                               m_read_buf;
    long long                                 m_read_time;

    std::mutex                                m_receive_callback_sync;
    ReceiveCallbackT                          m_receive_callback;
    std::atomic<int>                          m_receive_timeout;
    std::atomic<int>                          m_receive_time;

    typedef Util::CExpMap<size_t, size_t>     SampleHashMapT;
    SampleHashMapT                            m_sample_hash;

    std::mutex                                m_event_callback_map_sync;
    typedef std::map<eCAL_Subscriber_Event, SubEventCallbackT> EventCallbackMapT;
    EventCallbackMapT                         m_event_callback_map;

    std::atomic<long long>                    m_clock;
    long long                                 m_clock_old;
    std::chrono::steady_clock::time_point     m_rec_time;
    long                                      m_freq;

    std::set<long long>                       m_id_set;
    
    typedef std::unordered_map<std::string, long long> WriterCounterMapT;
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
