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

#include "ecal_subscriber_impl.h"
#include <ecal/config.h>
#include <ecal/log.h>
#include <ecal/process.h>
#include <ecal/time.h>

#if ECAL_CORE_REGISTRATION
#include "registration/ecal_registration_provider.h"
#endif

#include "ecal_subscriber_impl.h"
#include "ecal_global_accessors.h"

#include "readwrite/ecal_reader_layer.h"
#include "readwrite/ecal_transport_layer.h"

#if ECAL_CORE_TRANSPORT_UDP
#include "readwrite/udp/ecal_reader_udp.h"
#include "readwrite/config/builder/udp_attribute_builder.h"
#endif

#if ECAL_CORE_TRANSPORT_SHM
#include "readwrite/shm/ecal_reader_shm.h"
#include "readwrite/config/builder/shm_attribute_builder.h"
#endif

#if ECAL_CORE_TRANSPORT_TCP
#include "readwrite/tcp/ecal_reader_tcp.h"
#include "readwrite/config/builder/tcp_attribute_builder.h"
#endif

#include <algorithm>
#include <chrono>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>

namespace eCAL
{
  ////////////////////////////////////////
  // CSubscriberImpl
  ////////////////////////////////////////
  CSubscriberImpl::CSubscriberImpl(const SDataTypeInformation& topic_info_, const eCAL::eCALReader::SAttributes& attr_) :
                 m_topic_info(topic_info_),
                 m_topic_size(0),
                 m_receive_time(0),
                 m_clock(0),
                 m_frequency_calculator(3.0f),
                 m_created(false),
                 m_attributes(attr_)
  {
#ifndef NDEBUG
    // log it
    Logging::Log(Logging::log_level_debug1, m_attributes.topic_name + "::CSubscriberImpl::Constructor");
#endif

    // build subscriber id
    m_subscriber_id = std::chrono::steady_clock::now().time_since_epoch().count();

    // build topic id
    m_topic_id.topic_name = m_attributes.topic_name;
    m_topic_id.topic_id.entity_id = m_subscriber_id;
    m_topic_id.topic_id.host_name = m_attributes.host_name;
    m_topic_id.topic_id.process_id = m_attributes.process_id;

    // start transport layers
    InitializeLayers();
    StartTransportLayer();

    // mark as created
    m_created = true;
  }

  CSubscriberImpl::~CSubscriberImpl()
  {
#ifndef NDEBUG
    // log it
    Logging::Log(Logging::log_level_debug1, m_attributes.topic_name + "::CSubscriberImpl::Destructor");
#endif

    if (!m_created) return;

    // stop transport layers
    StopTransportLayer();

    // reset receive callback
    {
      const std::lock_guard<std::mutex> lock(m_receive_callback_mutex);
      m_receive_callback = nullptr;
    }

    // mark as no more created
    m_created = false;

    // and unregister
    Unregister();
  }

  bool CSubscriberImpl::Read(std::string& buf_, long long* time_ /* = nullptr */, int rcv_timeout_ms_ /* = 0 */)
  {
    if (!m_created) return(false);

    std::unique_lock<std::mutex> read_buffer_lock(m_read_buf_mutex);

    // No need to wait (for whatever time) if something has been received
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
      Logging::Log(Logging::log_level_debug3, m_attributes.topic_name + "::CSubscriberImpl::Read");
#endif
      // copy content to target string
      buf_.clear();
      buf_.swap(m_read_buf);
      m_read_buf_received = false;

      // apply time
      if (time_ != nullptr) *time_ = m_read_time;

      // return success
      return(true);
    }

    return(false);
  }

  bool CSubscriberImpl::SetReceiveCallback(const ReceiveCallbackT& callback_)
  {
    if (!m_created) return(false);

#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CSubscriberImpl::SetReceiveCallback");
#endif

    // set receive callback
    {
      const std::lock_guard<std::mutex> lock(m_receive_callback_mutex);
      m_receive_callback = callback_;
    }

    return(true);
  }

  bool CSubscriberImpl::RemoveReceiveCallback()
  {
    if (!m_created) return(false);

#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CSubscriberImpl::RemoveReceiveCallback");
#endif

    // remove receive callback
    {
      const std::lock_guard<std::mutex> lock(m_receive_callback_mutex);
      m_receive_callback = nullptr;
    }

    return(true);
  }

  bool CSubscriberImpl::SetEventCallback(const SubEventCallbackT& callback_)
  {
    if (!m_created) return false;

#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CSubscriberImpl::SetEventIDCallback");
#endif

    // set event id callback
    {
      const std::lock_guard<std::mutex> lock(m_event_id_callback_mutex);
      m_event_id_callback = callback_;
    }
    return true;
  }

  bool CSubscriberImpl::RemoveEventCallback()
  {
    if (!m_created) return false;

#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CSubscriberImpl::RemoveEventCallback");
#endif

    // remove event id callback
    {
      const std::lock_guard<std::mutex> lock(m_event_id_callback_mutex);
      m_event_id_callback = nullptr;
    }
    return true;
  }

  void CSubscriberImpl::SetFilterIDs(const std::set<long long>& filter_ids_)
  {
#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CSubscriberImpl::SetFilterIDs");
#endif

    m_id_set = filter_ids_;
  }

  void CSubscriberImpl::ApplyPublisherRegistration(const SPublicationInfo& publication_info_, const SDataTypeInformation& data_type_info_, const SLayerStates& pub_layer_states_)
  {
    // flag write enabled from publisher side (information not used yet)
#if ECAL_CORE_TRANSPORT_UDP
    m_layers.udp.write_enabled = pub_layer_states_.udp.write_enabled;
#endif
#if ECAL_CORE_TRANSPORT_SHM
    m_layers.shm.write_enabled = pub_layer_states_.shm.write_enabled;
#endif
#if ECAL_CORE_TRANSPORT_TCP
    m_layers.tcp.write_enabled = pub_layer_states_.tcp.write_enabled;
#endif

    // add key to connection map, including connection state
    bool is_new_connection = false;
    {
      const std::lock_guard<std::mutex> lock(m_connection_map_mtx);
      auto publication_info_iter = m_connection_map.find(publication_info_);

      if (publication_info_iter == m_connection_map.end())
      {
        // add publisher to connection map, connection state false
        m_connection_map[publication_info_] = SConnection{ data_type_info_, pub_layer_states_, false };
      }
      else
      {
        // existing connection, we got the second update now
        auto& connection = publication_info_iter->second;

        // if this connection was inactive before
        // activate it now and flag a new connection finally
        if (!connection.state)
        {
          is_new_connection = true;
        }

        // update the data type and layer states, even if the connection is not new
        connection = SConnection{ data_type_info_, pub_layer_states_, true };
      }

      // update connection count
      m_connection_count = GetConnectionCount();
    }

    // handle these events outside the lock
    if (is_new_connection)
    {
      // fire connect event
      FireConnectEvent(publication_info_, data_type_info_);
    }

#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug3, m_attributes.topic_name + "::CSubscriberImpl::ApplyPublisherRegistration");
#endif
  }

  void CSubscriberImpl::ApplyPublisherUnregistration(const SPublicationInfo& publication_info_, const SDataTypeInformation& data_type_info_)
  {
    // remove key from connection map
    {
      const std::lock_guard<std::mutex> lock(m_connection_map_mtx);

      m_connection_map.erase(publication_info_);

      // update connection count
      m_connection_count = GetConnectionCount();
    }

    // fire disconnect event
    FireDisconnectEvent(publication_info_, data_type_info_);
    
#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug3, m_attributes.topic_name + "::CSubscriberImpl::ApplyPublisherUnregistration");
#endif
  }

  void CSubscriberImpl::ApplyLayerParameter(const SPublicationInfo& publication_info_, eTLayerType type_, const Registration::ConnectionPar& parameter_)
  {
    SReaderLayerPar par;
    par.host_name  = publication_info_.host_name;
    par.process_id = publication_info_.process_id;
    par.topic_name = m_attributes.topic_name;
    par.topic_id   = publication_info_.entity_id;
    par.parameter  = parameter_;

    switch (type_)
    {
    case tl_ecal_shm:
#if ECAL_CORE_TRANSPORT_SHM
      CSHMReaderLayer::Get()->SetConnectionParameter(par);
#endif
      break;
    case tl_ecal_tcp:
#if ECAL_CORE_TRANSPORT_TCP
      CTCPReaderLayer::Get()->SetConnectionParameter(par);
#endif
      break;
    default:
      break;
    }
  }

  void CSubscriberImpl::InitializeLayers()
  {
    // initialize udp layer
#if ECAL_CORE_TRANSPORT_UDP
    if (m_attributes.udp.enable)
    {
      CUDPReaderLayer::Get()->Initialize(eCAL::eCALReader::BuildUDPAttributes(m_attributes));
    }
#endif

    // initialize shm layer
#if ECAL_CORE_TRANSPORT_SHM
    if (m_attributes.shm.enable)
    {
      CSHMReaderLayer::Get()->Initialize(eCAL::eCALReader::BuildSHMAttributes(m_attributes));
    }
#endif

    // initialize tcp layer
#if ECAL_CORE_TRANSPORT_TCP
    if (m_attributes.tcp.enable)
    {
      CTCPReaderLayer::Get()->Initialize(eCAL::eCALReader::BuildTCPLayerAttributes(m_attributes));
    }
#endif
  }

  size_t CSubscriberImpl::ApplySample(const Payload::TopicInfo& topic_info_, const char* payload_, size_t size_, long long id_, long long clock_, long long time_, size_t /*hash_*/, eTLayerType layer_)
  {
    // ensure thread safety
    const std::lock_guard<std::mutex> lock(m_receive_callback_mutex);
    if (!m_created) return(0);

    // We don't want to apply samples which are received on layers which are not activated for this subscriber
    if (!ShouldApplySampleBasedOnLayer(layer_))
    {
      return 0;
    }

    auto publication_info = PublicationInfoFromTopicInfo(topic_info_);

    // We do not want to apply duplicate / old samples
    if (!ShouldApplySampleBasedOnClock(publication_info, clock_))
    {
      // not clear why we are returning the size_ if we are not applying the sample, but why not...
      return size_;
    }

    // We might not want to apply samples sent with a given ID (deprecated!)
    if (!ShouldApplySampleBasedOnId(id_))
    {
      return 0;
    }

    // store receive layer
    m_layers.udp.active |= layer_ == tl_ecal_udp;
    m_layers.shm.active |= layer_ == tl_ecal_shm;
    m_layers.tcp.active |= layer_ == tl_ecal_tcp;

#ifndef NDEBUG
    // log it
    Logging::Log(Logging::log_level_debug3, m_attributes.topic_name + "::CSubscriberImpl::ApplySample");
#endif

    // increase read clock
    m_clock++;

    TriggerMessageDropUdate(publication_info, clock_);
    TriggerStatisticsUpdate(time_);

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
        Logging::Log(Logging::log_level_debug3, m_attributes.topic_name + "::CSubscriberImpl::ApplySample::ReceiveCallback");
#endif
        // prepare data struct
        SReceiveCallbackData cb_data;
        cb_data.buffer   = static_cast<const void*>(payload_);
        cb_data.buffer_size  = size_;
        cb_data.send_timestamp  = time_;
        cb_data.send_clock = clock_;

        STopicId topic_id;
        topic_id.topic_name          = topic_info_.topic_name;
        topic_id.topic_id.host_name  = topic_info_.host_name;
        topic_id.topic_id.entity_id  = topic_info_.topic_id;
        topic_id.topic_id.process_id = topic_info_.process_id;

        SPublicationInfo pub_info;
        pub_info.entity_id  = topic_info_.topic_id;
        pub_info.host_name  = topic_info_.host_name;
        pub_info.process_id = topic_info_.process_id;

        // execute it
        const std::lock_guard<std::mutex> exec_lock(m_connection_map_mtx);
        (m_receive_callback)(topic_id, m_connection_map[pub_info].data_type_info, cb_data);
        processed = true;
      }
    }

    // if not consumed by user receive call
    if (!processed)
    {
      // push sample into read buffer
      const std::lock_guard<std::mutex> read_buffer_lock(m_read_buf_mutex);
      m_read_buf.clear();
      m_read_buf.assign(payload_, payload_ + size_);
      m_read_time = time_;
      m_read_buf_received = true;

      // inform receive
      m_read_buf_cv.notify_one();
#ifndef NDEBUG
      // log it
      Logging::Log(Logging::log_level_debug3, m_attributes.topic_name + "::CSubscriberImpl::ApplySample::Receive::Buffered");
#endif
    }

    return(size_);
  }

  void CSubscriberImpl::Register()
  {
#if ECAL_CORE_REGISTRATION
    Registration::Sample sample;
    GetRegistrationSample(sample);
    auto registration_provider = g_registration_provider();
    if (registration_provider) registration_provider->RegisterSample(sample);

#ifndef NDEBUG
    // log it
    Logging::Log(Logging::log_level_debug4, m_attributes.topic_name + "::CSubscriberImpl::Register");
#endif
#endif // ECAL_CORE_REGISTRATION
  }

  void CSubscriberImpl::Unregister()
  {
#if ECAL_CORE_REGISTRATION
    Registration::Sample sample;
    GetUnregistrationSample(sample);
    auto registration_provider = g_registration_provider();
    if (registration_provider) registration_provider->UnregisterSample(sample);

#ifndef NDEBUG
    // log it
    Logging::Log(Logging::log_level_debug4, m_attributes.topic_name + "::CSubscriberImpl::Unregister");
#endif
#endif // ECAL_CORE_REGISTRATION
  }

  void CSubscriberImpl::GetRegistration(Registration::Sample& sample)
  {
    // return registration
    return GetRegistrationSample(sample);
  }

  bool CSubscriberImpl::IsPublished() const
  {
    return m_connection_count > 0;
  }

  size_t CSubscriberImpl::GetPublisherCount() const
  {
    return m_connection_count;
  }
    
  void CSubscriberImpl::GetRegistrationSample(Registration::Sample& ecal_reg_sample)
  {
    ecal_reg_sample.cmd_type = bct_reg_subscriber;

    auto& ecal_reg_sample_identifier = ecal_reg_sample.identifier;
    ecal_reg_sample_identifier.process_id = m_attributes.process_id;
    ecal_reg_sample_identifier.entity_id  = m_subscriber_id;
    ecal_reg_sample_identifier.host_name  = m_attributes.host_name;

    auto& ecal_reg_sample_topic                = ecal_reg_sample.topic;
    ecal_reg_sample_topic.shm_transport_domain = m_attributes.shm_transport_domain;
    ecal_reg_sample_topic.topic_name           = m_attributes.topic_name;
    // topic_information
    {
      auto& ecal_reg_sample_tdatatype      = ecal_reg_sample_topic.datatype_information;
      ecal_reg_sample_tdatatype.encoding   = m_topic_info.encoding;
      ecal_reg_sample_tdatatype.name       = m_topic_info.name;
      ecal_reg_sample_tdatatype.descriptor = m_topic_info.descriptor;
    }
    ecal_reg_sample_topic.topic_size = static_cast<int32_t>(m_topic_size);

#if ECAL_CORE_TRANSPORT_UDP
    // udp multicast layer
    {
      Registration::TLayer udp_tlayer;
      udp_tlayer.type      = tl_ecal_udp;
      udp_tlayer.version   = ecal_transport_layer_version;
      udp_tlayer.enabled   = m_layers.udp.read_enabled;
      udp_tlayer.active    = m_layers.udp.active;
      ecal_reg_sample_topic.transport_layer.push_back(udp_tlayer);
    }
#endif

#if ECAL_CORE_TRANSPORT_SHM
    // shm layer
    {
      Registration::TLayer shm_tlayer;
      shm_tlayer.type      = tl_ecal_shm;
      shm_tlayer.version   = ecal_transport_layer_version;
      shm_tlayer.enabled   = m_layers.shm.read_enabled;
      shm_tlayer.active    = m_layers.shm.active;
      ecal_reg_sample_topic.transport_layer.push_back(shm_tlayer);
    }
#endif

#if ECAL_CORE_TRANSPORT_TCP
    // tcp layer
    {
      Registration::TLayer tcp_tlayer;
      tcp_tlayer.type      = tl_ecal_tcp;
      tcp_tlayer.version   = ecal_transport_layer_version;
      tcp_tlayer.enabled   = m_layers.tcp.read_enabled;
      tcp_tlayer.active    = m_layers.tcp.active;
      ecal_reg_sample_topic.transport_layer.push_back(tcp_tlayer);
    }
#endif

    ecal_reg_sample_topic.process_name   = m_attributes.process_name;
    ecal_reg_sample_topic.unit_name      = m_attributes.unit_name;
    ecal_reg_sample_topic.data_clock     = m_clock;
    {
      const std::lock_guard<std::mutex> lock(m_statistics_mutex);
      ecal_reg_sample_topic.data_frequency = GetFrequency();
      ecal_reg_sample_topic.latency_us = m_latency_us_calculator.GetStatistics();
    }
    ecal_reg_sample_topic.message_drops  = GetMessageDropsAndFireDroppedEvents();

    // we do not know the number of connections ..
    ecal_reg_sample_topic.connections_local = 0;
    ecal_reg_sample_topic.connections_external = 0;

    //std::cout << "Latencies: (" << ecal_reg_sample_topic.latency_us.min << ", " << ecal_reg_sample_topic.latency_us.mean << ", " << ecal_reg_sample_topic.latency_us.max << ")\n";
  }

  void CSubscriberImpl::GetUnregistrationSample(Registration::Sample& ecal_unreg_sample)
  {
    ecal_unreg_sample.cmd_type = bct_unreg_subscriber;

    auto& ecal_reg_sample_identifier = ecal_unreg_sample.identifier;
    ecal_reg_sample_identifier.process_id = m_attributes.process_id;
    ecal_reg_sample_identifier.entity_id  = m_subscriber_id;
    ecal_reg_sample_identifier.host_name  = m_attributes.host_name;

    auto& ecal_reg_sample_topic                = ecal_unreg_sample.topic;
    ecal_reg_sample_topic.shm_transport_domain = m_attributes.shm_transport_domain;
    ecal_reg_sample_topic.process_name         = m_attributes.process_name;
    ecal_reg_sample_topic.topic_name           = m_attributes.topic_name;
    ecal_reg_sample_topic.unit_name            = m_attributes.unit_name;
  }
  
  void CSubscriberImpl::StartTransportLayer()
  {
#if ECAL_CORE_TRANSPORT_UDP
    if (m_attributes.udp.enable)
    {
      // flag enabled
      m_layers.udp.read_enabled = true;

      // subscribe to layer (if supported)
      CUDPReaderLayer::Get()->AddSubscription(m_attributes.host_name, m_attributes.topic_name, m_subscriber_id);
    }
#endif

#if ECAL_CORE_TRANSPORT_SHM
    if (m_attributes.shm.enable)
    {
      // flag enabled
      m_layers.shm.read_enabled = true;

      // subscribe to layer (if supported)
      CSHMReaderLayer::Get()->AddSubscription(m_attributes.host_name, m_attributes.topic_name, m_subscriber_id);
    }
#endif

#if ECAL_CORE_TRANSPORT_TCP
    if (m_attributes.tcp.enable)
    {
      // flag enabled
      m_layers.tcp.read_enabled = true;

      // subscribe to layer (if supported)
      CTCPReaderLayer::Get()->AddSubscription(m_attributes.host_name, m_attributes.topic_name, m_subscriber_id);
    }
#endif
  }
  
  void CSubscriberImpl::StopTransportLayer()
  {
#if ECAL_CORE_TRANSPORT_UDP
    if (m_attributes.udp.enable)
    {
      // flag disabled
      m_layers.udp.read_enabled = false;

      // unsubscribe from layer (if supported)
      CUDPReaderLayer::Get()->RemSubscription(m_attributes.host_name, m_attributes.topic_name, m_subscriber_id);
    }
#endif

#if ECAL_CORE_TRANSPORT_SHM
    if (m_attributes.shm.enable)
    {
      // flag disabled
      m_layers.shm.read_enabled = false;

      // unsubscribe from layer (if supported)
      CSHMReaderLayer::Get()->RemSubscription(m_attributes.host_name, m_attributes.topic_name, m_subscriber_id);
    }
#endif

#if ECAL_CORE_TRANSPORT_TCP
    if (m_attributes.tcp.enable)
    {
      // flag disabled
      m_layers.tcp.read_enabled = false;

      // unsubscribe from layer (if supported)
      CTCPReaderLayer::Get()->RemSubscription(m_attributes.host_name, m_attributes.topic_name, m_subscriber_id);
    }
#endif
  }

  void CSubscriberImpl::FireEvent(const eSubscriberEvent type_, const SPublicationInfo& publication_info_, const SDataTypeInformation& data_type_info_)
  {
    // new event handling with topic id
    if (m_event_id_callback)
    {
      SSubEventCallbackData data;
      data.event_type         = type_;
      data.event_time         = eCAL::Time::GetMicroSeconds();
      data.publisher_datatype = data_type_info_;

      STopicId topic_id;
      topic_id.topic_id.entity_id  = publication_info_.entity_id;
      topic_id.topic_id.process_id = publication_info_.process_id;
      topic_id.topic_id.host_name  = publication_info_.host_name;
      topic_id.topic_name          = m_attributes.topic_name;
      const std::lock_guard<std::mutex> lock(m_event_id_callback_mutex);

      // call event callback
      m_event_id_callback(topic_id, data);
    }
  }

  void CSubscriberImpl::FireConnectEvent(const SPublicationInfo& publication_info_, const SDataTypeInformation& data_type_info_)
  {
    FireEvent(eSubscriberEvent::connected, publication_info_, data_type_info_);
  }

  void CSubscriberImpl::FireDisconnectEvent(const SPublicationInfo& publication_info_, const SDataTypeInformation& data_type_info_)
  {
    FireEvent(eSubscriberEvent::disconnected, publication_info_, data_type_info_);
  }

  void CSubscriberImpl::FireDroppedEvent(const SPublicationInfo& publication_info_, const SDataTypeInformation& data_type_info_)
  {
    FireEvent(eSubscriberEvent::dropped, publication_info_, data_type_info_);
  }

  CSubscriberImpl::SPublicationInfo CSubscriberImpl::PublicationInfoFromTopicInfo(const Payload::TopicInfo& topic_info_)
  {
    SPublicationInfo publication_info;
    publication_info.entity_id = topic_info_.topic_id;
    publication_info.host_name = topic_info_.host_name;
    publication_info.process_id = topic_info_.process_id;
    return publication_info;
  }

  size_t CSubscriberImpl::GetConnectionCount()
  {
    // no need to lock map here for now, map locked by caller
    size_t count(0);
    for (const auto& sub : m_connection_map)
    {
      if (sub.second.state)
      {
        count++;
      }
    }
    return count;
  }

  bool CSubscriberImpl::ShouldApplySampleBasedOnClock(const SPublicationInfo& publication_info_, long long clock_) const
  {
    // If counter is already present (duplicate), or unsure if it was present, the sample is not applied
    if (m_publisher_message_counter_map.HasCounter(publication_info_, clock_) != CounterCacheMapT::CounterInCache::False)
    {
#ifndef NDEBUG
      // log it
      Logging::Log(Logging::log_level_debug3, m_attributes.topic_name + "::CSubscriberImpl::AddSample discard sample because of multiple receive");
#endif

      return false;
    }

    // The sample counter is strictly monotonically increasing. If not so, we received an old message.
    // If it is applied or not depends on the configuration. Anyways, a message at low debug level is logged.
    if (!m_publisher_message_counter_map.IsMonotonic(publication_info_, clock_))
    {
#ifndef NDEBUG
      std::string msg = "Subscriber: \'";
      msg += m_attributes.topic_name;
      msg += "\'";
      msg += " received a message in the wrong order";
      Logging::Log(Logging::log_level_warning, msg);
#endif

      // @TODO: We should not have a global config call here. This should be an attribute of the subscriber!
      if (Config::GetDropOutOfOrderMessages())
      {
        return false;
      }
      else
      {
        return true;
      }
    }

    return true;
  }

  bool CSubscriberImpl::ShouldApplySampleBasedOnId(long long id_) const
  {
    // 
    if (!m_id_set.empty() && (m_id_set.find(id_) == m_id_set.end()))
    {
      return false;
    }

    return true;
  }

  void CSubscriberImpl::TriggerStatisticsUpdate(long long send_time_)
  {
    const auto receive_time = eCAL::Time::ecal_clock::now();
    
    const std::lock_guard<std::mutex> freq_lock(m_statistics_mutex);
    m_frequency_calculator.addTick(receive_time);

    eCAL::Time::ecal_clock::time_point sent_time_point{ std::chrono::microseconds(send_time_)};
    auto latency_us = std::chrono::duration_cast<std::chrono::microseconds>(receive_time - sent_time_point).count();

    m_latency_us_calculator.Update(static_cast<double>(latency_us));
  }

  void CSubscriberImpl::TriggerMessageDropUdate(const SPublicationInfo& publication_info_, uint64_t message_counter)
  {
    const std::lock_guard<std::mutex> lock(m_message_drop_map_mutex);
    m_message_drop_map.RegisterReceivedMessage(publication_info_, message_counter);
  }

  bool CSubscriberImpl::ShouldApplySampleBasedOnLayer(eTLayerType layer_) const
  {
    // check receive layer configuration
    switch (layer_)
    {
    case tl_ecal_udp:
      if (!m_attributes.udp.enable) return false;
      break;
    case tl_ecal_shm:
      if (!m_attributes.shm.enable) return false;
      break;
    case tl_ecal_tcp:
      if (!m_attributes.tcp.enable) return false;
      break;
    default:
      break;
    }

    return true;
  }

  int32_t CSubscriberImpl::GetFrequency()
  {
    const auto frequency_time = eCAL::Time::ecal_clock::now();

    const double frequency_in_mhz = m_frequency_calculator.getFrequency(frequency_time) * 1000;

    if (frequency_in_mhz > static_cast<double>(std::numeric_limits<int32_t>::max())) {
      return std::numeric_limits<int32_t>::max();
    }
    else if (frequency_in_mhz < static_cast<double>(std::numeric_limits<int32_t>::min())) {
      return std::numeric_limits<int32_t>::min();
    }
    return static_cast<int32_t>(frequency_in_mhz);
  }

  int32_t CSubscriberImpl::GetMessageDropsAndFireDroppedEvents()
  {
    const std::lock_guard<std::mutex> lock_drops(m_message_drop_map_mutex);
    const std::lock_guard<std::mutex> lock_connections(m_connection_map_mtx);

    auto message_drop_summary_map = m_message_drop_map.GetSummary();
    int32_t accumulated_message_drops = 0;

    for (const auto& message_drop_summary_pair : message_drop_summary_map)
    {
      const auto& publisher_info = message_drop_summary_pair.first;
      const auto& message_drop_summary = message_drop_summary_pair.second;

      if (message_drop_summary.new_drops)
      {
        // @TODO: Firing dropped events should happen from a different thread, we should queue this somehow...
        FireDroppedEvent(publisher_info, m_connection_map[publisher_info].data_type_info);
      }

      accumulated_message_drops += message_drop_summary.drops;
    }

    return accumulated_message_drops;
  }
}