/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include <ecal/pubsub/types.h>
#include <ecal/v5/ecal_callback.h>

#include "serialization/ecal_serialize_sample_payload.h"
#include "serialization/ecal_serialize_sample_registration.h"
#include "util/frequency_calculator.h"
#include "util/message_drop_calculator.h"
#include "util/counter_cache.h"
#include "readwrite/config/attributes/reader_attributes.h"

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
#include <unordered_map>

namespace eCAL
{
  class CSubscriberImpl
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

    using SPublicationInfo = Registration::SampleIdentifier;

    CSubscriberImpl(const SDataTypeInformation& topic_info_, const eCAL::eCALReader::SAttributes& attr_);
    ~CSubscriberImpl();

    // Delete copy constructor and copy assignment operator
    CSubscriberImpl(const CSubscriberImpl&) = delete;
    CSubscriberImpl& operator=(const CSubscriberImpl&) = delete;

    // Delete move constructor and move assignment operator
    CSubscriberImpl(CSubscriberImpl&&) = delete;
    CSubscriberImpl& operator=(CSubscriberImpl&&) = delete;

    bool Read(std::string& buf_, long long* time_ = nullptr, int rcv_timeout_ms_ = 0);

    bool SetReceiveCallback(ReceiveCallbackT callback_);
    bool RemoveReceiveCallback();

    // deprecated event callback interface
    bool SetEventCallback(eSubscriberEvent type_, v5::SubEventCallbackT callback_);
    bool RemoveEventCallback(eSubscriberEvent type_);

    // future event callback interface
    bool SetEventIDCallback(const SubEventCallbackT callback_);
    bool RemoveEventCallback();

    bool SetAttribute(const std::string& attr_name_, const std::string& attr_value_);
    bool ClearAttribute(const std::string& attr_name_);

    void SetFilterIDs(const std::set<long long>& filter_ids_);

    void ApplyPublisherRegistration(const SPublicationInfo& publication_info_, const SDataTypeInformation& data_type_info_, const SLayerStates& pub_layer_states_);
    void ApplyPublisherUnregistration(const SPublicationInfo& publication_info_, const SDataTypeInformation& data_type_info_);

    void ApplyLayerParameter(const SPublicationInfo& publication_info_, eTLayerType type_, const Registration::ConnectionPar& parameter_);

    void GetRegistration(Registration::Sample& sample);
    bool IsCreated() const { return(m_created); }

    bool IsPublished() const;
    size_t GetPublisherCount() const;

    const STopicId& GetTopicId() const { return m_topic_id; }
    const std::string& GetTopicName() const { return(m_attributes.topic_name); }
    const SDataTypeInformation& GetDataTypeInformation() const { return(m_topic_info); }

    void InitializeLayers();
    size_t ApplySample(const Payload::TopicInfo& topic_info_, const char* payload_, size_t size_, long long id_, long long clock_, long long time_, size_t hash_, eTLayerType layer_);

  protected:
    void Register();
    void Unregister();

    void GetRegistrationSample(Registration::Sample& sample);
    void GetUnregistrationSample(Registration::Sample& sample);

    void StartTransportLayer();
    void StopTransportLayer();

    void FireEvent(const eSubscriberEvent type_, const SPublicationInfo& publication_info_, const SDataTypeInformation& data_type_info_);

    void FireConnectEvent   (const SPublicationInfo& publication_info_, const SDataTypeInformation& data_type_info_);
    void FireDisconnectEvent(const SPublicationInfo& publication_info_, const SDataTypeInformation& data_type_info_);
    void FireDroppedEvent   (const SPublicationInfo& publication_info_, const SDataTypeInformation& data_type_info_);

    static SPublicationInfo PublicationInfoFromTopicInfo(const Payload::TopicInfo& topic_info_);

    size_t GetConnectionCount();

    bool ShouldApplySampleBasedOnClock(const SPublicationInfo& publication_info_, long long clock_) const;
    bool ShouldApplySampleBasedOnLayer(eTLayerType layer_) const;
    bool ShouldApplySampleBasedOnId(long long id_) const;

    void TriggerFrequencyUpdate();
    void TriggerMessageDropUdate(const SPublicationInfo& publication_info_, uint64_t message_counter);

    int32_t GetFrequency();
    int32_t GetMessageDropsAndFireDroppedEvents();

    EntityIdT                                 m_subscriber_id;
    SDataTypeInformation                      m_topic_info;
    STopicId                                  m_topic_id;
    std::atomic<size_t>                       m_topic_size;

    struct SConnection
    {
      SDataTypeInformation data_type_info;
      SLayerStates         layer_states;
      bool                 state = false;
    };
    using ConnectionMapT = std::map<SPublicationInfo, SConnection>;
    mutable std::mutex                        m_connection_map_mtx;
    ConnectionMapT                            m_connection_map;
    std::atomic<size_t>                       m_connection_count{ 0 };

    mutable std::mutex                        m_read_buf_mutex;
    std::condition_variable                   m_read_buf_cv;
    bool                                      m_read_buf_received = false;
    std::string                               m_read_buf;
    long long                                 m_read_time = 0;

    std::mutex                                m_receive_callback_mutex;
    ReceiveCallbackT                          m_receive_callback;
    std::atomic<int>                          m_receive_time;

    std::deque<size_t>                        m_sample_hash_queue;

    using EventCallbackMapT = std::map<eSubscriberEvent, v5::SubEventCallbackT>;
    std::mutex                                m_event_callback_map_mutex;
    EventCallbackMapT                         m_event_callback_map;

    std::mutex                                m_event_id_callback_mutex;
    SubEventCallbackT                         m_event_id_callback;

    std::atomic<long long>                    m_clock;

    std::mutex                                m_frequency_calculator_mutex;
    ResettableFrequencyCalculator<std::chrono::steady_clock> m_frequency_calculator;

    std::mutex                                m_message_drop_map_mutex;
    using MessageDropMapT = MessageDropCalculatorMap<SPublicationInfo>;
    MessageDropMapT                           m_message_drop_map;
    
    using CounterCacheMapT = CounterCacheMap<SPublicationInfo>;
    CounterCacheMapT                          m_publisher_message_counter_map;
    
    std::set<long long>                       m_id_set;

    SLayerStates                              m_layers;
    std::atomic<bool>                         m_created;

    eCAL::eCALReader::SAttributes             m_attributes;

    std::mutex                                m_state_mutex;
  };
}
