/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

#include <ecal/ecal_callback.h>
#include <ecal/ecal_types.h>
#include <ecal/config/subscriber.h>

#include "serialization/ecal_serialize_sample_payload.h"
#include "serialization/ecal_serialize_sample_registration.h"
#include "util/ecal_expmap.h"
#include "util/frequency_calculator.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <map>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>

namespace eCAL
{
  class CDataReader
  {
  public:
    struct SLayerState
    {
      bool write_enabled = false;   // is publisher enabled to write data on this layer?
      bool read_enabled  = false;   // is this subscriber configured to read data from this layer?
      bool active        = false;   // data has been received on this layer
    };
 
    struct SLayerStates
    {
      SLayerState udp;
      SLayerState shm;
      SLayerState tcp;
    };

    struct SPublicationInfo
    {
      std::string host_name;
      int32_t     process_id = 0;
      std::string topic_id;

      friend bool operator<(const SPublicationInfo& l, const SPublicationInfo& r)
      {
        return std::tie(l.host_name, l.process_id, l.topic_id)
          < std::tie(r.host_name, r.process_id, r.topic_id);
      }
    };

    CDataReader(const std::string& topic_name_, const SDataTypeInformation& topic_info_, const Subscriber::Configuration& config_ = {});
    ~CDataReader();

    bool Stop();

    bool Read(std::string& buf_, long long* time_ = nullptr, int rcv_timeout_ms_ = 0);

    bool AddReceiveCallback(ReceiveCallbackT callback_);
    bool RemReceiveCallback();

    bool AddEventCallback(eCAL_Subscriber_Event type_, SubEventCallbackT callback_);
    bool RemEventCallback(eCAL_Subscriber_Event type_);

    bool SetAttribute(const std::string& attr_name_, const std::string& attr_value_);
    bool ClearAttribute(const std::string& attr_name_);

    void SetID(const std::set<long long>& id_set_);

    void ApplyPublication(const SPublicationInfo& publication_info_, const SDataTypeInformation& data_type_info_, const SLayerStates& layer_states_);
    void RemovePublication(const SPublicationInfo& publication_info_);

    void ApplyLayerParameter(const SPublicationInfo& publication_info_, eTLayerType type_, const Registration::ConnectionPar& parameter_);

    Registration::Sample GetRegistration();
    bool IsCreated() const { return(m_created); }

    bool IsPublished() const
    {
      std::lock_guard<std::mutex> const lock(m_pub_map_mtx);
      return(!m_pub_map.empty());
    }

    size_t GetPublisherCount() const
    {
      const std::lock_guard<std::mutex> lock(m_pub_map_mtx);
      return(m_pub_map.size());
    }

    std::string          GetTopicName()           const { return(m_topic_name); }
    std::string          GetTopicID()             const { return(m_topic_id); }
    SDataTypeInformation GetDataTypeInformation() const { return(m_topic_info); }

    void InitializeLayers();
    size_t ApplySample(const std::string& tid_, const char* payload_, size_t size_, long long id_, long long clock_, long long time_, size_t hash_, eTLayerType layer_);

    std::string Dump(const std::string& indent_ = "");

  protected:
    void Register();
    void Unregister();

    void CheckConnections();

    Registration::Sample GetRegistrationSample();
    Registration::Sample GetUnregistrationSample();

    void StartTransportLayer();
    void StopTransportLayer();

    void FireConnectEvent(const std::string& tid_, const SDataTypeInformation& tinfo_);
    void FireDisconnectEvent();

    bool CheckMessageClock(const std::string& tid_, long long current_clock_);

    int32_t GetFrequency();

    std::string                               m_host_name;
    std::string                               m_host_group_name;
    int                                       m_pid = 0;
    std::string                               m_pname;
    std::string                               m_topic_name;
    std::string                               m_topic_id;
    SDataTypeInformation                      m_topic_info;
    std::map<std::string, std::string>        m_attr;
    std::atomic<size_t>                       m_topic_size;
    Subscriber::Configuration                 m_config;

    std::atomic<bool>                         m_connected;
    using PublicationMapT = Util::CExpirationMap<SPublicationInfo, std::tuple<SDataTypeInformation, SLayerStates>>;
    mutable std::mutex                        m_pub_map_mtx;
    PublicationMapT                           m_pub_map;

    mutable std::mutex                        m_read_buf_mtx;
    std::condition_variable                   m_read_buf_cv;
    bool                                      m_read_buf_received = false;
    std::string                               m_read_buf;
    long long                                 m_read_time = 0;

    std::mutex                                m_receive_callback_mtx;
    ReceiveCallbackT                          m_receive_callback;
    std::atomic<int>                          m_receive_time;

    std::deque<size_t>                        m_sample_hash_queue;

    using EventCallbackMapT = std::map<eCAL_Subscriber_Event, SubEventCallbackT>;
    std::mutex                                m_event_callback_map_mtx;
    EventCallbackMapT                         m_event_callback_map;

    std::atomic<long long>                    m_clock;

    std::mutex                                m_frequency_calculator_mtx;
    ResettableFrequencyCalculator<std::chrono::steady_clock> m_frequency_calculator;

    std::set<long long>                       m_id_set;

    using WriterCounterMapT = std::unordered_map<std::string, long long>;
    WriterCounterMapT                         m_writer_counter_map;
    long long                                 m_message_drops = 0;

    bool                                      m_share_ttype = false;
    bool                                      m_share_tdesc = false;

    SLayerStates                              m_layers;
    std::atomic<bool>                         m_created;
  };
}
