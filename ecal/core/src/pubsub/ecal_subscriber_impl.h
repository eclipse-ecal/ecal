/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 * Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
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
#include <ecal/time.h>
#include <ecal/v5/ecal_callback.h>

#include "serialization/ecal_serialize_sample_payload.h"
#include "serialization/ecal_serialize_sample_registration.h"
#include "util/frequency_calculator.h"
#include "util/message_drop_calculator.h"
#include "util/statistics_calculator.h"
#include "util/counter_cache.h"
#include "readwrite/config/attributes/reader_attributes.h"
#include "readwrite/ecal_reader_layer.h"

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
    CSubscriberImpl(const SDataTypeInformation& topic_info_, const eCAL::eCALReader::SAttributes& attr_);
    ~CSubscriberImpl();

    // Delete copy constructor and copy assignment operator
    CSubscriberImpl(const CSubscriberImpl&) = delete;
    CSubscriberImpl& operator=(const CSubscriberImpl&) = delete;

    // Delete move constructor and move assignment operator
    CSubscriberImpl(CSubscriberImpl&&) = delete;
    CSubscriberImpl& operator=(CSubscriberImpl&&) = delete;

    bool Read(std::string& buf_, long long* time_ = nullptr, int rcv_timeout_ms_ = 0);

    bool SetReceiveCallback(const ReceiveCallbackT& callback_);
    bool RemoveReceiveCallback();

    bool SetEventCallback(const SubEventCallbackT& callback_);
    bool RemoveEventCallback();

    void SetFilterIDs(const std::set<long long>& filter_ids_);

    void GetRegistration(Registration::Sample& sample);
    bool IsCreated() const { return(m_created); }

    bool IsPublished() const;
    size_t GetPublisherCount() const;

    const STopicId& GetTopicId() const { return m_topic_id; }
    const std::string& GetTopicName() const { return(m_attributes.topic_name); }
    const SDataTypeInformation& GetDataTypeInformation() const { return(m_topic_info); }

  protected:
    void Register();
    void Unregister();

    void GetRegistrationSample(Registration::Sample& sample);
    void GetUnregistrationSample(Registration::Sample& sample);

    void StartTransportLayer();
    void StopTransportLayer();

    void FireEvent(const eSubscriberEvent type_, const STopicId& publication_info_, const SDataTypeInformation& data_type_info_);

    void FireConnectEvent(const STopicId& publication_info_, const SDataTypeInformation& data_type_info_);
    void FireDisconnectEvent(const STopicId& publication_info_, const SDataTypeInformation& data_type_info_);
    void FireDroppedEvent(const STopicId& publication_info_, const SDataTypeInformation& data_type_info_);

    void InternalDataCallback(const STopicId& publisher_id_, const SDataTypeInformation& data_type_info_, const SReceiveCallbackData& data_);
    bool ShouldApplySampleBasedOnId(long long id_) const;

    void TriggerStatisticsUpdate(long long send_time_);
    void TriggerMessageDropUdate(const STopicId& publication_info_, uint64_t message_counter);

    int32_t GetFrequency();
    int32_t GetMessageDropsAndFireDroppedEvents();

    EntityIdT                                 m_subscriber_id;
    SDataTypeInformation                      m_topic_info;
    STopicId                                  m_topic_id;
    std::atomic<size_t>                       m_topic_size;

    mutable std::mutex                        m_read_buf_mutex;
    std::condition_variable                   m_read_buf_cv;
    bool                                      m_read_buf_received = false;
    std::string                               m_read_buf;
    long long                                 m_read_time = 0;

    std::mutex                                m_receive_callback_mutex;
    ReceiveCallbackT                          m_receive_callback;
    std::atomic<int>                          m_receive_time;

    std::deque<size_t>                        m_sample_hash_queue;

    std::mutex                                m_event_id_callback_mutex;
    SubEventCallbackT                         m_event_id_callback;

    std::atomic<long long>                    m_clock;

    std::mutex                                m_statistics_mutex;
    ResettableFrequencyCalculator<eCAL::Time::ecal_clock> m_frequency_calculator;
    StatisticsCalculator                      m_latency_us_calculator;

    std::mutex                                m_message_drop_map_mutex;
    using MessageDropMapT = MessageDropCalculatorMap<STopicId>;
    MessageDropMapT                           m_message_drop_map;
      
    std::set<long long>                       m_id_set;

    std::atomic<bool>                         m_created;

    eCAL::eCALReader::SAttributes             m_attributes;
    eCAL::SubscriptionHandle                  m_reader_handle;

    mutable std::mutex                        m_connection_info_mutex;
    SubscriberConnectionInfo                  m_connection_info;
  };
}
