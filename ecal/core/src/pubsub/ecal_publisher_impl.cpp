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

#include <ecal/config.h>
#include <ecal/log.h>
#include <ecal/pubsub/payload_writer.h>
#include <ecal/process.h>
#include <ecal/time.h>

#if ECAL_CORE_REGISTRATION
#include "registration/ecal_registration_provider.h"
#endif

#include "ecal_publisher_impl.h"
#include "ecal_global_accessors.h"

#include "readwrite/ecal_writer_base.h"
#include "readwrite/ecal_writer_buffer_payload.h"
#include "readwrite/ecal_transport_layer.h"

#include "readwrite/config/builder/shm_attribute_builder.h"
#include "readwrite/config/builder/tcp_attribute_builder.h"
#include "readwrite/config/builder/udp_attribute_builder.h"

#include <algorithm>
#include <chrono>
#include <functional>
#include <limits>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>

struct SSndHash
{
  SSndHash(const eCAL::EntityIdT& t, long long c) : topic_id(t), snd_clock(c) {}
  eCAL::EntityIdT topic_id;
  long long                     snd_clock;
};

namespace std
{
  template<>
  class hash<SSndHash> {
  public:
    size_t operator()(const SSndHash& h) const
    {
      const size_t h1 = std::hash<eCAL::EntityIdT>()(h.topic_id);
      const size_t h2 = std::hash<long long>()(h.snd_clock);
      return h1 ^ (h2 << 1);
    }
  };
}

namespace
{
#ifndef NDEBUG
  // function to convert boolean to string
  std::string boolToString(bool value)
  {
    return value ? "true" : "false";
  }

  // function to log the states of SLayerState
  void logLayerState(const std::string& layerName, const eCAL::CPublisherImpl::SLayerState& state) {
    std::cout << layerName << " - Read Enabled: " << boolToString(state.read_enabled)
      << ", Write Enabled: " << boolToString(state.write_enabled)
      << ", Write Active : " << boolToString(state.active) << std::endl;
  }

  // function to log the states of SLayerStates
  void logLayerStates(const eCAL::CPublisherImpl::SLayerStates& states) {
    std::cout << "Logging Layer States:" << std::endl;
    logLayerState("UDP", states.udp);
    logLayerState("SHM", states.shm);
    logLayerState("TCP", states.tcp);
  }
#endif
}

namespace eCAL
{
  CPublisherImpl::CPublisherImpl(const SDataTypeInformation& topic_info_, const eCAL::eCALWriter::SAttributes& attr_) :
    m_topic_info(topic_info_),
    m_attributes(attr_),
    m_frequency_calculator(3.0f),
    m_created(false)
  {
#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CPublisherImpl::Constructor");
#endif

    // build topic id
    m_topic_id = std::chrono::steady_clock::now().time_since_epoch().count();

    // mark as created
    m_created = true;
  }

  CPublisherImpl::~CPublisherImpl()
  {
#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug1, m_attributes.topic_name + "::CPublisherImpl::Destructor");
#endif

    if (!m_created) return;

    // stop all transport layer
    StopAllLayer();

    // clear subscriber maps
    {
      const std::lock_guard<std::mutex> lock(m_connection_map_mutex);
      m_connection_map.clear();
    }

    // clear event callback map
    {
      const std::lock_guard<std::mutex> lock(m_event_callback_map_mutex);
      m_event_callback_map.clear();
    }

    // mark as no more created
    m_created = false;

    // and unregister
    Unregister();
  }

  bool CPublisherImpl::Write(CPayloadWriter& payload_, long long time_, long long filter_id_)
  {
    // get payload buffer size (one time, to avoid multiple computations)
    const size_t payload_buf_size(payload_.GetSize());

    // are we allowed to perform zero copy writing?
    bool allow_zero_copy(false);
#if ECAL_CORE_TRANSPORT_SHM
    allow_zero_copy = m_attributes.shm.zero_copy_mode; // zero copy mode activated by user
#endif
#if ECAL_CORE_TRANSPORT_UDP
    // udp is active -> no zero copy
    allow_zero_copy &= !m_writer_udp;
#endif
#if ECAL_CORE_TRANSPORT_TCP
    // tcp is active -> no zero copy
    allow_zero_copy &= !m_writer_tcp;
#endif

    // create a payload copy for all layer
    if (!allow_zero_copy)
    {
      m_payload_buffer.resize(payload_buf_size);
      payload_.WriteFull(m_payload_buffer.data(), m_payload_buffer.size());
    }

    // prepare counter and internal states
    const size_t snd_hash = PrepareWrite(filter_id_, payload_buf_size);

    // did we write anything
    bool written(false);

    ////////////////////////////////////////////////////////////////////////////
    // SHM
    ////////////////////////////////////////////////////////////////////////////
#if ECAL_CORE_TRANSPORT_SHM
    if (m_writer_shm)
    {
#ifndef NDEBUG
      Logging::Log(Logging::log_level_debug3, m_attributes.topic_name + "::CPublisherImpl::Write::SHM");
#endif

      // send it
      bool shm_sent(false);
      {
        // fill writer data
        struct SWriterAttr wattr;
        wattr.len = payload_buf_size;
        wattr.id = m_id;
        wattr.clock = m_clock;
        wattr.hash = snd_hash;
        wattr.time = time_;
        wattr.zero_copy = m_attributes.shm.zero_copy_mode;
        wattr.acknowledge_timeout_ms = m_attributes.shm.acknowledge_timeout_ms;

        // prepare send
        if (m_writer_shm->PrepareWrite(wattr))
        {
          // register new to update listening subscribers and rematch
          Register();
          Process::SleepMS(5);
        }

        // we are the only active layer, and we support zero copy -> we do a zero copy write via payload
        if (allow_zero_copy)
        {
          // write to shm layer (write content into the opened memory file without additional copy)
          shm_sent = m_writer_shm->Write(payload_, wattr);
        }
        // multiple layer are active -> we make a copy and use that one
        else
        {
          // wrap the buffer into a payload object
          CBufferPayloadWriter payload_buf(m_payload_buffer.data(), m_payload_buffer.size());
          // write to shm layer (write content into the opened memory file without additional copy)
          shm_sent = m_writer_shm->Write(payload_buf, wattr);
        }

        m_layers.shm.active = true;
      }
      written |= shm_sent;

#ifndef NDEBUG
      if (shm_sent)
      {
        Logging::Log(Logging::log_level_debug3, m_attributes.topic_name + "::CPublisherImpl::Write::SHM - SUCCESS");
      }
      else
      {
        Logging::Log(Logging::log_level_error, m_attributes.topic_name + "::CPublisherImpl::Write::SHM - FAILED");
      }
#endif
    }
#endif // ECAL_CORE_TRANSPORT_SHM

    ////////////////////////////////////////////////////////////////////////////
    // UDP (MC)
    ////////////////////////////////////////////////////////////////////////////
#if ECAL_CORE_TRANSPORT_UDP
    if (m_writer_udp)
    {
#ifndef NDEBUG
      Logging::Log(Logging::log_level_debug3, m_attributes.topic_name + "::CPublisherImpl::Write::udp");
#endif

      // send it
      bool udp_sent(false);
      {
        // fill writer data
        struct SWriterAttr wattr;
        wattr.len = payload_buf_size;
        wattr.id = m_id;
        wattr.clock = m_clock;
        wattr.hash = snd_hash;
        wattr.time = time_;
        wattr.loopback = m_attributes.loopback;

        // prepare send
        if (m_writer_udp->PrepareWrite(wattr))
        {
          // register new to update listening subscribers and rematch
          Register();
          Process::SleepMS(5);
        }

        // write to udp multicast layer
        udp_sent = m_writer_udp->Write(m_payload_buffer.data(), wattr);
        m_layers.udp.active = true;
      }
      written |= udp_sent;

#ifndef NDEBUG
      if (udp_sent)
      {
        Logging::Log(Logging::log_level_debug3, m_attributes.topic_name + "::CPublisherImpl::Write::udp - SUCCESS");
      }
      else
      {
        Logging::Log(Logging::log_level_error, m_attributes.topic_name + "::CPublisherImpl::Write::udp - FAILED");
      }
#endif
    }
#endif // ECAL_CORE_TRANSPORT_UDP

    ////////////////////////////////////////////////////////////////////////////
    // TCP
    ////////////////////////////////////////////////////////////////////////////
#if ECAL_CORE_TRANSPORT_TCP
    if (m_writer_tcp)
    {
#ifndef NDEBUG
      Logging::Log(Logging::log_level_debug3, m_attributes.topic_name + "::CPublisherImpl::Send::TCP");
#endif

      // send it
      bool tcp_sent(false);
      {
        // fill writer data
        struct SWriterAttr wattr;
        wattr.len = payload_buf_size;
        wattr.id = m_id;
        wattr.clock = m_clock;
        wattr.hash = snd_hash;
        wattr.time = time_;

        // write to tcp layer
        tcp_sent = m_writer_tcp->Write(m_payload_buffer.data(), wattr);
        m_layers.tcp.active = true;
      }
      written |= tcp_sent;

#ifndef NDEBUG
      if (tcp_sent)
      {
        Logging::Log(Logging::log_level_debug3, m_attributes.topic_name + "::CPublisherImpl::Write::TCP - SUCCESS");
      }
      else
      {
        Logging::Log(eCAL::Logging::log_level_error, m_attributes.topic_name + "::CPublisherImpl::Write::TCP - FAILED");
      }
#endif
    }
#endif // ECAL_CORE_TRANSPORT_TCP

    // return success
    return written;
  }

  bool CPublisherImpl::SetDataTypeInformation(const SDataTypeInformation& topic_info_)
  {
    m_topic_info = topic_info_;

#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CPublisherImpl::SetDataTypeInformation");
#endif

    return(true);
  }

  bool CPublisherImpl::SetAttribute(const std::string& /* attr_name_ */_, const std::string& /* attr_value_ */)
  {
#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CPublisherImpl::SetAttribute");
#endif
    Logging::Log(Logging::log_level_warning, m_attributes.topic_name + "::CPublisherImpl::SetAttribute - Setting publisher attributes no longer has an effect.");
    
    return(false);
  }

  bool CPublisherImpl::ClearAttribute(const std::string& /* attr_name_ */)
  {
#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CPublisherImpl::ClearAttribute");
#endif
    Logging::Log(Logging::log_level_warning, m_attributes.topic_name + "::CPublisherImpl::ClearAttribute - Clear publisher attributes no longer has an effect.");

    return(false);
  }

  bool CPublisherImpl::SetEventCallback(ePublisherEvent type_, const v5::PubEventCallbackT callback_)
  {
    if (!m_created) return(false);

#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CPublisherImpl::SetEventCallback");
#endif

    // set event callback
    {
      const std::lock_guard<std::mutex> lock(m_event_callback_map_mutex);
      m_event_callback_map[type_] = callback_;
    }

    return(true);
  }

  bool CPublisherImpl::RemoveEventCallback(ePublisherEvent type_)
  {
    if (!m_created) return(false);

#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CPublisherImpl::RemoveEventCallback");
#endif

    // remove event callback
    {
      const std::lock_guard<std::mutex> lock(m_event_callback_map_mutex);
      m_event_callback_map[type_] = nullptr;
    }

    return(true);
  }

  bool CPublisherImpl::SetEventCallback(const PubEventCallbackT callback_)
  {
    if (!m_created) return false;

#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CPublisherImpl::SetEventCallback");
#endif

    // set event id callback
    {
      const std::lock_guard<std::mutex> lock(m_event_id_callback_mutex);
      m_event_id_callback = callback_;
    }
    return true;
  }

  bool CPublisherImpl::RemoveEventCallback()
  {
    if (!m_created) return false;

#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CPublisherImpl::RemoveEventCallback");
#endif

    // remove event id callback
    {
      const std::lock_guard<std::mutex> lock(m_event_id_callback_mutex);
      m_event_id_callback = nullptr;
    }
    return true;
  }

  void CPublisherImpl::ApplySubscriberRegistration(const SSubscriptionInfo& subscription_info_, const SDataTypeInformation& data_type_info_, const SLayerStates& sub_layer_states_, const std::string& reader_par_)
  {
    // collect layer states
    std::vector<eTLayerType> pub_layers;
    std::vector<eTLayerType> sub_layers;
#if ECAL_CORE_TRANSPORT_UDP
    if (m_attributes.udp.enable)            pub_layers.push_back(tl_ecal_udp);
    if (sub_layer_states_.udp.read_enabled) sub_layers.push_back(tl_ecal_udp);

    m_layers.udp.read_enabled = sub_layer_states_.udp.read_enabled; // just for debugging/logging
#endif
#if ECAL_CORE_TRANSPORT_SHM
    if (m_attributes.shm.enable)            pub_layers.push_back(tl_ecal_shm);
    if (sub_layer_states_.shm.read_enabled) sub_layers.push_back(tl_ecal_shm);

    m_layers.shm.read_enabled = sub_layer_states_.shm.read_enabled; // just for debugging/logging
#endif
#if ECAL_CORE_TRANSPORT_TCP
    if (m_attributes.tcp.enable)            pub_layers.push_back(tl_ecal_tcp);
    if (sub_layer_states_.tcp.read_enabled) sub_layers.push_back(tl_ecal_tcp);

    m_layers.tcp.read_enabled = sub_layer_states_.tcp.read_enabled; // just for debugging/logging
#endif

    // determine if we need to start a transport layer
    const TransportLayer::eType layer2activate = DetermineTransportLayer2Start(pub_layers, sub_layers, m_attributes.host_name == subscription_info_.host_name);
    switch (layer2activate)
    {
    case TransportLayer::eType::udp_mc:
      StartUdpLayer();
      break;
    case TransportLayer::eType::shm:
      StartShmLayer();
      break;
    case TransportLayer::eType::tcp:
      StartTcpLayer();
      break;
    default:
      break;
    }

#ifndef NDEBUG
    //logLayerStates(m_layers);
#endif

    // add a new subscription
#if ECAL_CORE_TRANSPORT_UDP
    if (m_writer_udp) m_writer_udp->ApplySubscription(subscription_info_.host_name, subscription_info_.process_id, subscription_info_.entity_id, reader_par_);
#endif
#if ECAL_CORE_TRANSPORT_SHM
    if (m_writer_shm) m_writer_shm->ApplySubscription(subscription_info_.host_name, subscription_info_.process_id, subscription_info_.entity_id, reader_par_);
#endif
#if ECAL_CORE_TRANSPORT_TCP
    if (m_writer_tcp) m_writer_tcp->ApplySubscription(subscription_info_.host_name, subscription_info_.process_id, subscription_info_.entity_id, reader_par_);
#endif

    // add key to connection map, including connection state
    bool is_new_connection = false;
    {
      const std::lock_guard<std::mutex> lock(m_connection_map_mutex);
      auto subscription_info_iter = m_connection_map.find(subscription_info_);

      if (subscription_info_iter == m_connection_map.end())
      {
        // add subscriber to connection map, connection state false
        m_connection_map[subscription_info_] = SConnection{ data_type_info_, sub_layer_states_, false };
      }
      else
      {
        // existing connection, we got the second update now
        auto& connection = subscription_info_iter->second;

        // if this connection was inactive before
        // activate it now and flag a new connection finally
        if (!connection.state)
        {
          is_new_connection = true;
        }

        // update the data type, the layer states and set the state active
        connection = SConnection{ data_type_info_, sub_layer_states_, true };
      }

      // update connection count
      m_connection_count = GetConnectionCount();
    }


    // handle these events outside the lock
    if (is_new_connection)
    {
      // fire connect event
      FireConnectEvent(subscription_info_, data_type_info_);
    }

#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug3, m_attributes.topic_name + "::CPublisherImpl::ApplySubscriberRegistration");
#endif
  }

  void CPublisherImpl::ApplySubscriberUnregistration(const SSubscriptionInfo& subscription_info_, const SDataTypeInformation& data_type_info_)
  {
    // remove subscription
#if ECAL_CORE_TRANSPORT_UDP
    if (m_writer_udp) m_writer_udp->RemoveSubscription(subscription_info_.host_name, subscription_info_.process_id, subscription_info_.entity_id);
#endif
#if ECAL_CORE_TRANSPORT_SHM
    if (m_writer_shm) m_writer_shm->RemoveSubscription(subscription_info_.host_name, subscription_info_.process_id, subscription_info_.entity_id);
#endif
#if ECAL_CORE_TRANSPORT_TCP
    if (m_writer_tcp) m_writer_tcp->RemoveSubscription(subscription_info_.host_name, subscription_info_.process_id, subscription_info_.entity_id);
#endif

    {
      const std::lock_guard<std::mutex> lock(m_connection_map_mutex);

      // remove key from connection map
      m_connection_map.erase(subscription_info_);

      // update connection count
      m_connection_count = GetConnectionCount();
    }

    // fire disconnect event
    FireDisconnectEvent(subscription_info_, data_type_info_);

#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug3, m_attributes.topic_name + "::CPublisherImpl::ApplySubscriberUnregistration");
#endif
  }

  void CPublisherImpl::RefreshSendCounter()
  {
    // increase write clock
    m_clock++;

    // update send frequency
    {
      // we should think about if we would like to potentially use the `time_` variable to tick with (but we would need the same base for checking incoming samples then....
      const auto send_time = std::chrono::steady_clock::now();
      const std::lock_guard<std::mutex> lock(m_frequency_calculator_mutex);
      m_frequency_calculator.addTick(send_time);
    }
  }

  bool CPublisherImpl::IsSubscribed() const
  {
    return m_connection_count > 0;
  }

  size_t CPublisherImpl::GetSubscriberCount() const
  {
    return m_connection_count;
  }

  void CPublisherImpl::Register()
  {
#if ECAL_CORE_REGISTRATION
    Registration::Sample registration_sample;
    GetRegistrationSample(registration_sample);
    if (g_registration_provider() != nullptr) g_registration_provider()->RegisterSample(registration_sample);

#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug4, m_attributes.topic_name + "::CPublisherImpl::Register");
#endif
#endif // ECAL_CORE_REGISTRATION
  }

  void CPublisherImpl::Unregister()
  {
#if ECAL_CORE_REGISTRATION
    Registration::Sample unregistration_sample;
    GetUnregistrationSample(unregistration_sample);
    if (g_registration_provider() != nullptr) g_registration_provider()->UnregisterSample(unregistration_sample);

#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug4, m_attributes.topic_name + "::CPublisherImpl::Unregister");
#endif
#endif // ECAL_CORE_REGISTRATION
  }

  void CPublisherImpl::GetRegistration(Registration::Sample& sample)
  {
    GetRegistrationSample(sample);
  }

  void CPublisherImpl::GetRegistrationSample(Registration::Sample& ecal_reg_sample)
  {
    ecal_reg_sample.cmd_type = bct_reg_publisher;

    auto& ecal_reg_sample_identifier = ecal_reg_sample.identifier;
    ecal_reg_sample_identifier.process_id = m_attributes.process_id;
    ecal_reg_sample_identifier.entity_id = m_topic_id;
    ecal_reg_sample_identifier.host_name = m_attributes.host_name;

    auto& ecal_reg_sample_topic = ecal_reg_sample.topic;
    ecal_reg_sample_topic.shm_transport_domain = m_attributes.shm_transport_domain;
    ecal_reg_sample_topic.topic_name           = m_attributes.topic_name;

    // topic_information
    {
      auto& ecal_reg_sample_tdatatype = ecal_reg_sample_topic.datatype_information;
      ecal_reg_sample_tdatatype.encoding   = m_topic_info.encoding;
      ecal_reg_sample_tdatatype.name       = m_topic_info.name;
      ecal_reg_sample_tdatatype.descriptor = m_topic_info.descriptor;
    }
    ecal_reg_sample_topic.topic_size = static_cast<int32_t>(m_topic_size);

#if ECAL_CORE_TRANSPORT_UDP
    // udp multicast layer
    if (m_writer_udp)
    {
      eCAL::Registration::TLayer udp_tlayer;
      udp_tlayer.type = tl_ecal_udp;
      udp_tlayer.version = ecal_transport_layer_version;
      udp_tlayer.enabled = m_layers.udp.write_enabled;
      udp_tlayer.active = m_layers.udp.active;
      udp_tlayer.par_layer.layer_par_udpmc = m_writer_udp->GetConnectionParameter().layer_par_udpmc;
      ecal_reg_sample_topic.transport_layer.push_back(udp_tlayer);
    }
#endif

#if ECAL_CORE_TRANSPORT_SHM
    // shm layer
    if (m_writer_shm)
    {
      eCAL::Registration::TLayer shm_tlayer;
      shm_tlayer.type = tl_ecal_shm;
      shm_tlayer.version = ecal_transport_layer_version;
      shm_tlayer.enabled = m_layers.shm.write_enabled;
      shm_tlayer.active = m_layers.shm.active;
      shm_tlayer.par_layer.layer_par_shm = m_writer_shm->GetConnectionParameter().layer_par_shm;
      ecal_reg_sample_topic.transport_layer.push_back(shm_tlayer);
    }
#endif

#if ECAL_CORE_TRANSPORT_TCP
    // tcp layer
    if (m_writer_tcp)
    {
      eCAL::Registration::TLayer tcp_tlayer;
      tcp_tlayer.type = tl_ecal_tcp;
      tcp_tlayer.version = ecal_transport_layer_version;
      tcp_tlayer.enabled = m_layers.tcp.write_enabled;
      tcp_tlayer.active = m_layers.tcp.active;
      tcp_tlayer.par_layer.layer_par_tcp = m_writer_tcp->GetConnectionParameter().layer_par_tcp;
      ecal_reg_sample_topic.transport_layer.push_back(tcp_tlayer);
    }
#endif

    ecal_reg_sample_topic.process_name = m_attributes.process_name;
    ecal_reg_sample_topic.unit_name    = m_attributes.unit_name;
    ecal_reg_sample_topic.data_id      = m_id;
    ecal_reg_sample_topic.data_clock       = m_clock;
    ecal_reg_sample_topic.data_frequency        = GetFrequency();

    size_t loc_connections(0);
    size_t ext_connections(0);
    {
      const std::lock_guard<std::mutex> lock(m_connection_map_mutex);
      for (const auto& sub : m_connection_map)
      {
        if (sub.first.host_name == m_attributes.host_name)
        {
          loc_connections++;
        }
      }
      ext_connections = m_connection_map.size() - loc_connections;
    }
    ecal_reg_sample_topic.connections_local = static_cast<int32_t>(loc_connections);
    ecal_reg_sample_topic.connections_external = static_cast<int32_t>(ext_connections);
  }

  void CPublisherImpl::GetUnregistrationSample(Registration::Sample& ecal_unreg_sample)
  {
    ecal_unreg_sample.cmd_type = bct_unreg_publisher;

    auto& ecal_reg_sample_identifier = ecal_unreg_sample.identifier;
    ecal_reg_sample_identifier.process_id = m_attributes.process_id;
    ecal_reg_sample_identifier.entity_id = m_topic_id;
    ecal_reg_sample_identifier.host_name = m_attributes.host_name;

    auto& ecal_reg_sample_topic = ecal_unreg_sample.topic;
    ecal_reg_sample_topic.shm_transport_domain = m_attributes.shm_transport_domain;
    ecal_reg_sample_topic.process_name         = m_attributes.process_name;
    ecal_reg_sample_topic.topic_name           = m_attributes.topic_name;
    ecal_reg_sample_topic.unit_name            = m_attributes.unit_name;
  }

  void CPublisherImpl::FireEvent(const ePublisherEvent type_, const SSubscriptionInfo& subscription_info_, const SDataTypeInformation& data_type_info_)
  {
    // new event handling with topic id
    if (m_event_id_callback)
    {
      SPubEventCallbackData data;
      data.event_type = type_;
      data.event_time = eCAL::Time::GetMicroSeconds();
      data.subscriber_datatype = data_type_info_;

      STopicId topic_id;
      topic_id.topic_id.entity_id = subscription_info_.entity_id;
      topic_id.topic_id.process_id = subscription_info_.process_id;
      topic_id.topic_id.host_name = subscription_info_.host_name;
      topic_id.topic_name = m_attributes.topic_name;
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
        v5::SPubEventCallbackData event_data;
        event_data.type = type_;
        event_data.time = eCAL::Time::GetMicroSeconds();
        event_data.clock = 0;
        event_data.tid = std::to_string(subscription_info_.entity_id);
        event_data.tdatatype = data_type_info_;

        // call event callback
        (iter->second)(m_attributes.topic_name.c_str(), &event_data);
      }
    }
  }

  void CPublisherImpl::FireConnectEvent(const SSubscriptionInfo& subscription_info_, const SDataTypeInformation& data_type_info_)
  {
    FireEvent(ePublisherEvent::connected, subscription_info_, data_type_info_);
  }

  void CPublisherImpl::FireDisconnectEvent(const SSubscriptionInfo& subscription_info_, const SDataTypeInformation& data_type_info_)
  {
    FireEvent(ePublisherEvent::disconnected, subscription_info_, data_type_info_);
  }

  size_t CPublisherImpl::GetConnectionCount()
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

  bool CPublisherImpl::StartUdpLayer()
  {
#if ECAL_CORE_TRANSPORT_UDP
    if (m_layers.udp.write_enabled) return false;

    // flag enabled
    m_layers.udp.write_enabled = true;

    // log state
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CPublisherImpl::StartUdpLayer::ACTIVATED");

    // create writer
    m_writer_udp = std::make_unique<CDataWriterUdpMC>(eCAL::eCALWriter::BuildUDPAttributes(m_topic_id, m_attributes));

    // register activated layer
    Register();

#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CPublisherImpl::StartUdpLayer::WRITER_CREATED");
#endif
    return true;
#else  // ECAL_CORE_TRANSPORT_UDP
    return false;
#endif // ECAL_CORE_TRANSPORT_UDP
  }

  bool CPublisherImpl::StartShmLayer()
  {
#if ECAL_CORE_TRANSPORT_SHM
    if (m_layers.shm.write_enabled) return false;

    // flag enabled
    m_layers.shm.write_enabled = true;

    // log state
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CPublisherImpl::StartShmLayer::ACTIVATED");

    // create writer
    m_writer_shm = std::make_unique<CDataWriterSHM>(eCAL::eCALWriter::BuildSHMAttributes(m_attributes));

    // register activated layer
    Register();

#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CPublisherImpl::StartShmLayer::WRITER_CREATED");
#endif
    return true;
#else  // ECAL_CORE_TRANSPORT_SHM
    return false;
#endif // ECAL_CORE_TRANSPORT_SHM
  }

  bool CPublisherImpl::StartTcpLayer()
  {
#if ECAL_CORE_TRANSPORT_TCP
    if (m_layers.tcp.write_enabled) return false;

    // flag enabled
    m_layers.tcp.write_enabled = true;

    // log state
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CPublisherImpl::StartTcpLayer::ACTIVATED");

    // create writer
    m_writer_tcp = std::make_unique<CDataWriterTCP>(eCAL::eCALWriter::BuildTCPAttributes(m_topic_id, m_attributes));

    // register activated layer
    Register();

#ifndef NDEBUG
    Logging::Log(Logging::log_level_debug2, m_attributes.topic_name + "::CPublisherImpl::StartTcpLayer::WRITER_CREATED");
#endif
    return true;
#else  // ECAL_CORE_TRANSPORT_TCP
    return false;
#endif // ECAL_CORE_TRANSPORT_TCP
  }

  void CPublisherImpl::StopAllLayer()
  {
#if ECAL_CORE_TRANSPORT_UDP
    // flag disabled
    m_layers.udp.write_enabled = false;

    // destroy writer
    m_writer_udp.reset();
#endif

#if ECAL_CORE_TRANSPORT_SHM
    // flag disabled
    m_layers.shm.write_enabled = false;

    // destroy writer
    m_writer_shm.reset();
#endif

#if ECAL_CORE_TRANSPORT_TCP
    // flag disabled
    m_layers.tcp.write_enabled = false;

    // destroy writer
    m_writer_tcp.reset();
#endif
  }

  size_t CPublisherImpl::PrepareWrite(long long id_, size_t len_)
  {
    // store id
    m_id = id_;

    // handle write counters
    RefreshSendCounter();

    // calculate unique send hash
    const std::hash<SSndHash> hf;
    const size_t snd_hash = hf(SSndHash(m_topic_id, m_clock));

    // store size for monitoring
    m_topic_size = len_;

    // return the hash for the write action
    return snd_hash;
  }

  TransportLayer::eType CPublisherImpl::DetermineTransportLayer2Start(const std::vector<eTLayerType>& enabled_pub_layer_, const std::vector<eTLayerType>& enabled_sub_layer_, bool same_host_)
  {
    // determine the priority list to use
    const Publisher::Configuration::LayerPriorityVector& layer_priority_vector = same_host_ ? m_attributes.layer_priority_local : m_attributes.layer_priority_remote;

    // find the highest priority transport layer that is available in both publisher and subscriber options
    static const std::map<TransportLayer::eType, eTLayerType> transport_layer_mapping {
      {TransportLayer::eType::none, tl_none },
      {TransportLayer::eType::shm, tl_ecal_shm},
      {TransportLayer::eType::udp_mc, tl_ecal_udp},
      {TransportLayer::eType::tcp, tl_ecal_tcp},
    };

    for (const TransportLayer::eType layer : layer_priority_vector)
    {
      if (std::find(enabled_pub_layer_.begin(), enabled_pub_layer_.end(), transport_layer_mapping.at(layer)) != enabled_pub_layer_.end()
        && std::find(enabled_sub_layer_.begin(), enabled_sub_layer_.end(), transport_layer_mapping.at(layer)) != enabled_sub_layer_.end())
      {
        return layer;
      }
    }

    // return tl_none if no common transport layer is found
    return TransportLayer::eType::none;
  }

  int32_t CPublisherImpl::GetFrequency()
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
