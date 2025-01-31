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

    // build topic id
    m_topic_id = std::chrono::steady_clock::now().time_since_epoch().count();

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

    // reset event callback map
    {
      const std::lock_guard<std::mutex> lock(m_event_callback_map_mutex);
      m_event_callback_map.clear();
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

  bool CSubscriberImpl::SetReceiveCallback(ReceiveCallbackT callback_)
  {
    if (!m_created) return(false);

#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CSubscriberImpl::SetReceiveCallback");
#endif

    // set receive callback
    {
      const std::lock_guard<std::mutex> lock(m_receive_callback_mutex);
      m_receive_callback = std::move(callback_);
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

  bool CSubscriberImpl::SetEventCallback(eSubscriberEvent type_, v5::SubEventCallbackT callback_)
  {
    if (!m_created) return(false);

#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CSubscriberImpl::SetEventCallback");
#endif

    // set event callback
    {
      const std::lock_guard<std::mutex> lock(m_event_callback_map_mutex);
      m_event_callback_map[type_] = std::move(callback_);
    }

    return(true);
  }

  bool CSubscriberImpl::RemoveEventCallback(eSubscriberEvent type_)
  {
    if (!m_created) return(false);

#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CSubscriberImpl::RemoveEventCallback");
#endif

    // remove event callback
    {
      const std::lock_guard<std::mutex> lock(m_event_callback_map_mutex);
      m_event_callback_map[type_] = nullptr;
    }

    return(true);
  }

  bool CSubscriberImpl::SetEventIDCallback(const SubEventCallbackT callback_)
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

  bool CSubscriberImpl::SetAttribute(const std::string& /* attr_name_ */, const std::string& /* attr_value_ */)
  {
#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CSubscriberImpl::SetAttribute");
#endif

    Logging::Log(Logging::log_level_warning, m_attributes.topic_name + "::CSubscriberImpl::SetAttribute - Setting subscriber attributes no longer has an effect.");

    return(false);
  }

  bool CSubscriberImpl::ClearAttribute(const std::string& /* attr_name_ */)
  {
#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CSubscriberImpl::ClearAttribute");
#endif

    Logging::Log(Logging::log_level_warning, m_attributes.topic_name + "::CSubscriberImpl::ClearAttribute - Clear subscriber attributes no longer has an effect.");

    return(false);
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
    bool last_connection_gone(false);
    {
      const std::lock_guard<std::mutex> lock(m_connection_map_mtx);

      m_connection_map.erase(publication_info_);
      last_connection_gone = m_connection_map.empty();

      // update connection count
      m_connection_count = GetConnectionCount();
    }

    if (last_connection_gone)
    {
      // fire disconnect event
      FireDisconnectEvent(publication_info_, data_type_info_);
    }

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

  size_t CSubscriberImpl::ApplySample(const Payload::TopicInfo& topic_info_, const char* payload_, size_t size_, long long id_, long long clock_, long long time_, size_t hash_, eTLayerType layer_)
  {
    // ensure thread safety
    const std::lock_guard<std::mutex> lock(m_receive_callback_mutex);
    if (!m_created) return(0);

    // check receive layer configuration
    switch (layer_)
    {
    case tl_ecal_udp:
      if (!m_attributes.udp.enable) return 0;
      break;
    case tl_ecal_shm:
      if (!m_attributes.shm.enable) return 0;
      break;
    case tl_ecal_tcp:
      if (!m_attributes.tcp.enable) return 0;
      break;
    default:
      break;
    }

    // store receive layer
    m_layers.udp.active |= layer_ == tl_ecal_udp;
    m_layers.shm.active |= layer_ == tl_ecal_shm;
    m_layers.tcp.active |= layer_ == tl_ecal_tcp;

    // number of hash values to track for duplicates
    constexpr int hash_queue_size(64);

    // use hash to discard multiple receives of the same payload
    //   if a hash is in the queue we received this message recently (on another transport layer ?)
    //   so we return and do not process this sample again
    if(std::find(m_sample_hash_queue.begin(), m_sample_hash_queue.end(), hash_) != m_sample_hash_queue.end())
    {
#ifndef NDEBUG
      // log it
      Logging::Log(Logging::log_level_debug3, m_attributes.topic_name + "::CSubscriberImpl::AddSample discard sample because of multiple receive");
#endif
      return(size_);
    }
    //   this is a new sample -> store its hash
    m_sample_hash_queue.push_back(hash_);

    // limit size of hash queue to the last 64 messages
    while (m_sample_hash_queue.size() > hash_queue_size) m_sample_hash_queue.pop_front();

    // check id
    // TODO: not sure if this is needed / necessary.
    if (!m_id_set.empty())
    {
      if (m_id_set.find(id_) == m_id_set.end()) return(0);
    }

    // check the current message clock
    // if the function returns false we detected
    //  - a dropped message
    //  - an out-of-order message
    //  - a multiple sent message
    SPublicationInfo publication_info;
    publication_info.entity_id  = topic_info_.topic_id;
    publication_info.host_name  = topic_info_.host_name;
    publication_info.process_id = topic_info_.process_id;
    if (!CheckMessageClock(publication_info, clock_))
    {
      // we will not process that message
      return(0);
    }

#ifndef NDEBUG
    // log it
    Logging::Log(Logging::log_level_debug3, m_attributes.topic_name + "::CSubscriberImpl::ApplySample");
#endif

    // increase read clock
    m_clock++;

    // Update frequency calculation
    {
      const auto receive_time = std::chrono::steady_clock::now();
      const std::lock_guard<std::mutex> freq_lock(m_frequency_calculator_mutex);
      m_frequency_calculator.addTick(receive_time);
    }

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
    if (g_registration_provider() != nullptr) g_registration_provider()->RegisterSample(sample);

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
    if (g_registration_provider() != nullptr) g_registration_provider()->UnregisterSample(sample);

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
    ecal_reg_sample_identifier.entity_id  = m_topic_id;
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
    ecal_reg_sample_topic.data_frequency = GetFrequency();
    ecal_reg_sample_topic.message_drops  = static_cast<int32_t>(m_message_drops);

    // we do not know the number of connections ..
    ecal_reg_sample_topic.connections_local = 0;
    ecal_reg_sample_topic.connections_external = 0;
  }

  void CSubscriberImpl::GetUnregistrationSample(Registration::Sample& ecal_unreg_sample)
  {
    ecal_unreg_sample.cmd_type = bct_unreg_subscriber;

    auto& ecal_reg_sample_identifier = ecal_unreg_sample.identifier;
    ecal_reg_sample_identifier.process_id = m_attributes.process_id;
    ecal_reg_sample_identifier.entity_id  = m_topic_id;
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
      CUDPReaderLayer::Get()->AddSubscription(m_attributes.host_name, m_attributes.topic_name, m_topic_id);
    }
#endif

#if ECAL_CORE_TRANSPORT_SHM
    if (m_attributes.shm.enable)
    {
      // flag enabled
      m_layers.shm.read_enabled = true;

      // subscribe to layer (if supported)
      CSHMReaderLayer::Get()->AddSubscription(m_attributes.host_name, m_attributes.topic_name, m_topic_id);
    }
#endif

#if ECAL_CORE_TRANSPORT_TCP
    if (m_attributes.tcp.enable)
    {
      // flag enabled
      m_layers.tcp.read_enabled = true;

      // subscribe to layer (if supported)
      CTCPReaderLayer::Get()->AddSubscription(m_attributes.host_name, m_attributes.topic_name, m_topic_id);
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
      CUDPReaderLayer::Get()->RemSubscription(m_attributes.host_name, m_attributes.topic_name, m_topic_id);
    }
#endif

#if ECAL_CORE_TRANSPORT_SHM
    if (m_attributes.shm.enable)
    {
      // flag disabled
      m_layers.shm.read_enabled = false;

      // unsubscribe from layer (if supported)
      CSHMReaderLayer::Get()->RemSubscription(m_attributes.host_name, m_attributes.topic_name, m_topic_id);
    }
#endif

#if ECAL_CORE_TRANSPORT_TCP
    if (m_attributes.tcp.enable)
    {
      // flag disabled
      m_layers.tcp.read_enabled = false;

      // unsubscribe from layer (if supported)
      CTCPReaderLayer::Get()->RemSubscription(m_attributes.host_name, m_attributes.topic_name, m_topic_id);
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

    // deprecated event handling with topic name
    {
      const std::lock_guard<std::mutex> lock(m_event_callback_map_mutex);
      auto iter = m_event_callback_map.find(type_);
      if (iter != m_event_callback_map.end() && iter->second)
      {
        v5::SSubEventCallbackData data;
        data.type      = type_;
        data.time      = eCAL::Time::GetMicroSeconds();
        data.clock     = 0;
        data.tid       = std::to_string(publication_info_.entity_id);
        data.tdatatype = data_type_info_;

        // call event callback
        (iter->second)(m_attributes.topic_name.c_str(), &data);
      }
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

  bool CSubscriberImpl::CheckMessageClock(const SPublicationInfo& publication_info_, long long current_clock_)
  {
    auto iter = m_writer_counter_map.find(publication_info_.entity_id);
    
    // initial entry
    if (iter == m_writer_counter_map.end())
    {
      m_writer_counter_map[publication_info_.entity_id] = current_clock_;
      return true;
    }
    // clock entry exists
    else
    {
      // calculate difference
      const long long last_clock = iter->second;
      const long long clock_difference = current_clock_ - last_clock;

      // this is perfect, the next message arrived
      if (clock_difference == 1)
      {
        // update the internal clock counter
        iter->second = current_clock_;

        // process it
        return true;
      }

      // that should never happen, maybe there is a publisher
      // sending parallel on multiple layers ?
      // we ignore this message duplicate
      if (clock_difference == 0)
      {
        // do not update the internal clock counter

        // do not process it
        return false;
      }

      // that means we miss at least one message
      // -> we have a "message drop"
      if (clock_difference > 1)
      {
#if 0
        // we log this
        std::string msg = std::to_string(counter_ - counter_last) + " Messages lost ! ";
        msg += "(Unit: \'";
        msg += m_attributes.unit_name;
        msg += "@";
        msg += m_attributes.host_name;
        msg += "\' | Subscriber: \'";
        msg += m_attributes.topic_name;
        msg += "\')";
        Logging::Log(log_level_warning, msg);
#endif

        // fire dropped event
        // we do not know the data type of the dropped message here
        // so we use an empty data type information
        FireDroppedEvent(publication_info_, SDataTypeInformation());

        // increase the drop counter
        m_message_drops += clock_difference;

        // update the internal clock counter
        iter->second = current_clock_;

        // process it
        return true;
      }

      // a negative clock difference may happen if a publisher
      // is using a shm ringbuffer and messages arrive in the wrong order
      if (clock_difference < 0)
      {
        // -----------------------------------
        // drop messages in the wrong order
        // -----------------------------------
        if (Config::GetDropOutOfOrderMessages())
        {
          // do not update the internal clock counter

          // there is no need to fire the drop event, because
          // this event has been fired with the message before

          // do not process it
          return false;
        }
        // -----------------------------------
        // process messages in the wrong order
        // -----------------------------------
        else
        {
          // do not update the internal clock counter

          // but we log this
          std::string msg = "Subscriber: \'";
          msg += m_attributes.topic_name;
          msg += "\'";
          msg += " received a message in the wrong order";
          Logging::Log(Logging::log_level_warning, msg);

          // process it
          return true;
        }
      }
    }

    // should never be reached
    return false;
  }

  int32_t CSubscriberImpl::GetFrequency()
  {
    const auto frequency_time = std::chrono::steady_clock::now();
    const std::lock_guard<std::mutex> lock(m_frequency_calculator_mutex);

    const double frequency_in_mhz = m_frequency_calculator.getFrequency(frequency_time) * 1000;

    if (frequency_in_mhz > static_cast<double>(std::numeric_limits<int32_t>::max())) {
      return std::numeric_limits<int32_t>::max();
    }
    else if (frequency_in_mhz < static_cast<double>(std::numeric_limits<int32_t>::min())) {
      return std::numeric_limits<int32_t>::min();
    }
    return static_cast<int32_t>(frequency_in_mhz);
  }
}
