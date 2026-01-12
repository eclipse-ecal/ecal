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

#include "readwrite/ecal_reader_manager.h"

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

  /*
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
  */

  /*
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

  */

  //size_t CSubscriberImpl::ApplySample(const Payload::TopicInfo& topic_info_, const char* payload_, size_t size_, long long id_, long long clock_, long long time_, size_t /*hash_*/, eTLayerType layer_)
  void CSubscriberImpl::InternalDataCallback(const STopicId& publisher_id_, const SDataTypeInformation& data_type_info_, const SReceiveCallbackData& data_)
  {
    if (!m_created) return;

    // ensure thread safety
    const std::lock_guard<std::mutex> lock(m_receive_callback_mutex);
       
    // We might not want to apply samples sent with a given ID (deprecated!)
    // TODO
    /*
    if (!ShouldApplySampleBasedOnId(id_))
    {
      return 0;
    }
    */

#ifndef NDEBUG
    // log it
    Logging::Log(Logging::log_level_debug3, m_attributes.topic_name + "::CSubscriberImpl::ApplySample");
#endif

    // increase read clock
    m_clock++;

    TriggerMessageDropUdate(publisher_id_, data_.send_clock);
    TriggerStatisticsUpdate(data_.send_timestamp);

    // reset timeout
    m_receive_time = 0;

    // store size
    m_topic_size = data_.buffer_size;

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

        // execute it
        (m_receive_callback)(publisher_id_, data_type_info_, data_);
        processed = true;
      }
    }

    // if not consumed by user receive call
    if (!processed)
    {
      // push sample into read buffer
      const std::lock_guard<std::mutex> read_buffer_lock(m_read_buf_mutex);
      m_read_buf.clear();
      m_read_buf.assign((char*) data_.buffer, (char*)data_.buffer + data_.buffer_size);
      // TODO: this is a bit weird, shouldn't we store the current time?
      m_read_time = data_.send_timestamp;
      m_read_buf_received = true;

      // inform receive
      m_read_buf_cv.notify_one();
#ifndef NDEBUG
      // log it
      Logging::Log(Logging::log_level_debug3, m_attributes.topic_name + "::CSubscriberImpl::ApplySample::Receive::Buffered");
#endif
    }
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
    std::lock_guard<std::mutex> lock(m_connection_info_mutex);
    return m_connection_info.IsConnected();
  }

  size_t CSubscriberImpl::GetPublisherCount() const
  {
    std::lock_guard<std::mutex> lock(m_connection_info_mutex);
    return m_connection_info.ConnectionCount();
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
      udp_tlayer.enabled   = m_attributes.udp.enable;
      //udp_tlayer.active    = m_layers.udp.active;
      ecal_reg_sample_topic.transport_layer.push_back(udp_tlayer);
    }
#endif

#if ECAL_CORE_TRANSPORT_SHM
    // shm layer
    {
      Registration::TLayer shm_tlayer;
      shm_tlayer.type      = tl_ecal_shm;
      shm_tlayer.version   = ecal_transport_layer_version;
      shm_tlayer.enabled   = m_attributes.shm.enable;
      //shm_tlayer.active    = m_layers.shm.active;
      ecal_reg_sample_topic.transport_layer.push_back(shm_tlayer);
    }
#endif

#if ECAL_CORE_TRANSPORT_TCP
    // tcp layer
    {
      Registration::TLayer tcp_tlayer;
      tcp_tlayer.type      = tl_ecal_tcp;
      tcp_tlayer.version   = ecal_transport_layer_version;
      tcp_tlayer.enabled   = m_attributes.tcp.enable;
      //tcp_tlayer.active    = m_layers.tcp.active;
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
    ActiveLayers layers;
    if (m_attributes.udp.enable) layers.set(LayerType::UDP);
    if (m_attributes.shm.enable) layers.set(LayerType::SHM);
    if (m_attributes.tcp.enable) layers.set(LayerType::TCP);

    //TODO
    //  TODO: fill subscription parameters
    SSubscriptionParameters subscription_parameters;
    subscription_parameters.topic.topic_id.entity_id = m_subscriber_id;
    subscription_parameters.topic.topic_id.process_id = m_attributes.process_id;
    subscription_parameters.topic.topic_id.host_name = m_attributes.host_name;
    subscription_parameters.topic.topic_name = m_attributes.topic_name;
    subscription_parameters.layers = layers;
        
    subscription_parameters.data_callback = [this](const STopicId& publisher_id_, const SDataTypeInformation& data_type_info_, const SReceiveCallbackData& data_)
    {
      this->InternalDataCallback(publisher_id_, data_type_info_, data_);
    };

    subscription_parameters.connection_changed_callback = [this](SubscriberConnectionChange change_, const SubscriberConnectionInstance& instance_)
    {
      switch (change_)
      {
      case SubscriberConnectionChange::NewConnection:
      {
        {
          std::lock_guard<std::mutex> lock(this->m_connection_info_mutex);
          this->m_connection_info.AddConnection(instance_);
        }
        this->FireConnectEvent(instance_.publisher_id, instance_.publisher_datatype_info);
      }
      break;
      case SubscriberConnectionChange::RemovedConnection:
        {
          {
            std::lock_guard<std::mutex> lock(this->m_connection_info_mutex);
            this->m_connection_info.RemoveConnection(instance_);
          }
          this->FireDisconnectEvent(instance_.publisher_id, instance_.publisher_datatype_info);
        }
        break;
      default:
        break;
      }
      };

    m_reader_handle = g_reader_manager()->AddSubscription(subscription_parameters);
  }
  
  void CSubscriberImpl::StopTransportLayer()
  {
    if (m_reader_handle) {
      g_reader_manager()->RemoveSubscription(*m_reader_handle);
      m_reader_handle.reset();
    }
  }

  void eCAL::CSubscriberImpl::FireEvent(const eSubscriberEvent type_, const STopicId& publication_info_, const SDataTypeInformation& data_type_info_)
  {
    // new event handling with topic id
    if (m_event_id_callback)
    {
      SSubEventCallbackData data;
      data.event_type = type_;
      data.event_time         = eCAL::Time::GetMicroSeconds();
      data.publisher_datatype = data_type_info_;

      const std::lock_guard<std::mutex> lock(m_event_id_callback_mutex);
      // call event callback
      m_event_id_callback(publication_info_, data);
    }
  }

  void CSubscriberImpl::FireConnectEvent(const STopicId& publication_info_, const SDataTypeInformation& data_type_info_)
  {
    FireEvent(eSubscriberEvent::connected, publication_info_, data_type_info_);
  }

  void CSubscriberImpl::FireDisconnectEvent(const STopicId& publication_info_, const SDataTypeInformation& data_type_info_)
  {
    FireEvent(eSubscriberEvent::disconnected, publication_info_, data_type_info_);
  }

  void CSubscriberImpl::FireDroppedEvent(const STopicId& publication_info_, const SDataTypeInformation& data_type_info_)
  {
    FireEvent(eSubscriberEvent::dropped, publication_info_, data_type_info_);
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
    const auto receive_time_us = eCAL::Time::GetMicroSeconds();
    const eCAL::Time::ecal_clock::time_point receive_time_clock{ std::chrono::microseconds(send_time_) };
    
    const std::lock_guard<std::mutex> freq_lock(m_statistics_mutex);
    m_frequency_calculator.addTick(receive_time_clock);

    auto latency_us = receive_time_us - send_time_;

    m_latency_us_calculator.Update(static_cast<double>(latency_us));
  }

  void CSubscriberImpl::TriggerMessageDropUdate(const STopicId& publication_info_, uint64_t message_counter)
  {
    const std::lock_guard<std::mutex> lock(m_message_drop_map_mutex);
    m_message_drop_map.RegisterReceivedMessage(publication_info_, message_counter);
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

    auto message_drop_summary_map = m_message_drop_map.GetSummary();
    int32_t accumulated_message_drops = 0;

    for (const auto& message_drop_summary_pair : message_drop_summary_map)
    {
      const auto& publisher_info = message_drop_summary_pair.first;
      const auto& message_drop_summary = message_drop_summary_pair.second;

      if (message_drop_summary.new_drops)
      {
        // @TODO: Firing dropped events should happen from a different thread, we should queue this somehow...
        // TODO: We no longer have information about potentially dropped messages
        FireDroppedEvent(publisher_info, {});
      }

      accumulated_message_drops += message_drop_summary.drops;
    }

    return accumulated_message_drops;
  }
}