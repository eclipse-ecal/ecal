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
 * @brief  eCAL publisher implementation
**/

#pragma once

#include <ecal/pubsub/types.h>
#include <ecal/pubsub/payload_writer.h>
#include <ecal/config.h>
#include <ecal/v5/ecal_callback.h>

#include "serialization/ecal_serialize_sample_registration.h"
#include "util/frequency_calculator.h"
#include "readwrite/config/attributes/writer_attributes.h"

#if ECAL_CORE_TRANSPORT_UDP
#include "readwrite/udp/ecal_writer_udp.h"
#endif

#if ECAL_CORE_TRANSPORT_SHM
#include "readwrite/shm/ecal_writer_shm.h"
#endif

#if ECAL_CORE_TRANSPORT_TCP
#include "readwrite/tcp/ecal_writer_tcp.h"
#endif

#include <atomic>
#include <chrono>
#include <cstddef>
#include <memory>
#include <mutex>
#include <string>
#include <map>
#include <vector>

namespace eCAL
{
  class CPublisherImpl
  {
  public:
    struct SLayerState
    {
      bool read_enabled  = false;   // is subscriber enabled to read data on this layer?
      bool write_enabled = false;   // is this publisher configured to write data from this layer?
      bool active        = false;   // data has been sent on this layer
    };
 
    struct SLayerStates
    {
      SLayerState udp;
      SLayerState shm;
      SLayerState tcp;
    };

    using SSubscriptionInfo = Registration::SampleIdentifier;

    CPublisherImpl(const SDataTypeInformation& topic_info_, const eCAL::eCALWriter::SAttributes& attr_);
    ~CPublisherImpl();

    bool Write(CPayloadWriter& payload_, long long time_, long long filter_id_);

    bool SetDataTypeInformation(const SDataTypeInformation& topic_info_);

    // deprecated event callback interface
    bool SetEventCallback(ePublisherEvent type_, const v5::PubEventCallbackT callback_);
    bool RemoveEventCallback(ePublisherEvent type_);

    // future event callback interface
    bool SetEventCallback(const PubEventCallbackT callback_);
    bool RemoveEventCallback();

    bool SetAttribute(const std::string& attr_name_, const std::string& attr_value_);
    bool ClearAttribute(const std::string& attr_name_);

    void ApplySubscriberRegistration(const SSubscriptionInfo& subscription_info_, const SDataTypeInformation& data_type_info_, const SLayerStates& sub_layer_states_, const std::string& reader_par_);
    void ApplySubscriberUnregistration(const SSubscriptionInfo& subscription_info_, const SDataTypeInformation& data_type_info_);

    void GetRegistration(Registration::Sample& sample);
    void RefreshSendCounter();

    bool IsCreated() const { return(m_created); }

    bool IsSubscribed() const;
    size_t GetSubscriberCount() const;

    STopicId GetTopicId() const
    {
      STopicId id;
      id.topic_name          = m_attributes.topic_name;
      id.topic_id.entity_id  = m_topic_id;
      id.topic_id.host_name  = m_attributes.host_name;
      id.topic_id.process_id = m_attributes.process_id;
      return id;
    }

    const std::string&          GetTopicName()           const { return(m_attributes.topic_name); }
    const SDataTypeInformation& GetDataTypeInformation() const { return m_topic_info; }

  protected:
    void Register();
    void Unregister();

    void GetRegistrationSample(Registration::Sample& sample);
    void GetUnregistrationSample(Registration::Sample& sample);

    bool StartUdpLayer();
    bool StartShmLayer();
    bool StartTcpLayer();

    void StopAllLayer();

    void FireEvent(const ePublisherEvent type_, const SSubscriptionInfo& subscription_info_, const SDataTypeInformation& data_type_info_);

    void FireConnectEvent   (const SSubscriptionInfo& subscription_info_, const SDataTypeInformation& data_type_info_);
    void FireDisconnectEvent(const SSubscriptionInfo& subscription_info_, const SDataTypeInformation& data_type_info_);

    size_t GetConnectionCount();

    size_t PrepareWrite(long long id_, size_t len_);

    TransportLayer::eType DetermineTransportLayer2Start(const std::vector<eTLayerType>& enabled_pub_layer_, const std::vector<eTLayerType>& enabled_sub_layer_, bool same_host_);
    
    int32_t GetFrequency();

    EntityIdT                m_topic_id;
    SDataTypeInformation                   m_topic_info;
    size_t                                 m_topic_size = 0;
    eCAL::eCALWriter::SAttributes          m_attributes;

    std::vector<char>                      m_payload_buffer;

    struct SConnection
    {
      SDataTypeInformation data_type_info;
      SLayerStates         layer_states;
      bool                 state = false;
    };
    using SSubscriptionMapT = std::map<SSubscriptionInfo, SConnection>;
    mutable std::mutex                     m_connection_map_mutex;
    SSubscriptionMapT                      m_connection_map;
    std::atomic<size_t>                    m_connection_count{ 0 };

    using EventCallbackMapT = std::map<ePublisherEvent, v5::PubEventCallbackT>;
    std::mutex                             m_event_callback_map_mutex;
    EventCallbackMapT                      m_event_callback_map;

    std::mutex                             m_event_id_callback_mutex;
    PubEventCallbackT                  m_event_id_callback;

    long long                              m_id = 0;
    long long                              m_clock = 0;

    std::mutex                             m_frequency_calculator_mutex;
    ResettableFrequencyCalculator<std::chrono::steady_clock> m_frequency_calculator;

#if ECAL_CORE_TRANSPORT_UDP
    std::unique_ptr<CDataWriterUdpMC>      m_writer_udp;
#endif
#if ECAL_CORE_TRANSPORT_SHM
    std::unique_ptr<CDataWriterSHM>        m_writer_shm;
#endif
#if ECAL_CORE_TRANSPORT_TCP
    std::unique_ptr<CDataWriterTCP>        m_writer_tcp;
#endif

    SLayerStates                           m_layers;
    std::atomic<bool>                      m_created;
  };
}
