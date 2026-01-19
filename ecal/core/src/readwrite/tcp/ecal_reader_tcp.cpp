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
 * @brief  tcp reader and layer
**/

#include "ecal_reader_tcp.h"

#include <cassert>

#include "ecal_tcp_pubsub_logger.h"
#include "ecal_utils/portable_endian.h"
#include <ecal_serialize_sample_payload.h>

namespace eCAL
{
  CDataReaderTCP::CDataReaderTCP(const eCAL::eCALReader::TCP::SAttributes& attr_, const STCPConnectionParameters& conn_params_, std::shared_ptr<tcp_pubsub::Executor>& executor_, DataCallback on_data_)
    : m_attributes(attr_)
    , m_on_data(on_data_)
    , m_subscriber(std::make_unique<tcp_pubsub::Subscriber>(executor_))
  {
    // The port must be valid
    assert(conn_params.port != 0);

    // Add possible hostnames:
    // 1. hostname:port
    // 2. hostname.local:port (-> i.e. the mDNS variant)
    const auto& host_name_ = conn_params_.host_name;
    const auto& port_ = conn_params_.port;
    const std::vector<std::pair<std::string, uint16_t>> publishers = { { host_name_, port_ } , {host_name_ + ".local", port_} };
    m_subscriber->addSession(publishers, m_attributes.max_reconnection_attempts);
    // TODO: is it not possible to miss data since we're setting the callback after adding the session?
    m_subscriber->setCallback(std::bind(&CDataReaderTCP::OnTcpMessage, this, std::placeholders::_1));
  }

  void CDataReaderTCP::OnTcpMessage(const tcp_pubsub::CallbackData& data_)
  {
    //                             ECAL                    + header size field
    const size_t   header_length = m_attributes.ecal_magic + sizeof(uint16_t);
    const uint16_t header_size   = le16toh(*reinterpret_cast<uint16_t*>(data_.buffer_->data() + m_attributes.ecal_magic));

    // extract header
    const char* header_payload = data_.buffer_->data() + header_length;
    // extract data payload
    const char* data_payload   = header_payload + header_size;

    // parse header
    if (DeserializeFromBuffer(header_payload, header_size, m_ecal_header))
    {
      // use this intermediate variables as optimization
      const auto& ecal_header_topic_info = m_ecal_header.topic_info;
      const auto& ecal_header_content    = m_ecal_header.content;

      SReceiveCallbackData callback_data_;
      callback_data_.buffer = data_payload;
      callback_data_.buffer_size = static_cast<size_t>(ecal_header_content.size);
      callback_data_.send_clock = ecal_header_content.clock;
      callback_data_.send_timestamp = ecal_header_content.time;
      // id & hash currently unused
      m_on_data(callback_data_);
    }
  }
  
  CTCPReaderLayer::CTCPReaderLayer(const eCAL::eCALReader::TCP::SAttributes& attr_)
    : CTransportLayerInstance(LayerType::TCP)
    , m_attributes(attr_)
  {
    const tcp_pubsub::logger::logger_t tcp_pubsub_logger = std::bind(TcpPubsubLogger, std::placeholders::_1, std::placeholders::_2);
    m_executor = std::make_shared<tcp_pubsub::Executor>(m_attributes.thread_pool_size, tcp_pubsub_logger);
  }

  bool CTCPReaderLayer::AcceptsConnection(const PublisherConnectionParameters& publisher, const SubscriberConnectionParameters& subscriber) const
  {
    return false;
  }

  CTransportLayerInstance::ConnectionToken CTCPReaderLayer::AddConnection(const PublisherConnectionParameters& publisher, const ReceiveCallbackT& on_data, const ConnectionChangeCallback& on_connection_changed)
  {
    STCPConnectionParameters conn_params;
    conn_params.host_name = publisher.GetHostName();
    conn_params.port     = static_cast<uint16_t>(publisher.GetLayerParameter(LayerType::TCP).par_layer.layer_par_tcp.port);
    
    STopicId publisher_topic_id = publisher.GetTopicId();
    SDataTypeInformation publisher_datatype_info = publisher.GetDataTypeInformation();
    auto data_callback = [on_data, publisher_topic_id, publisher_datatype_info](const SReceiveCallbackData& data_)
    {
      on_data(publisher_topic_id, publisher_datatype_info, data_);
    };

    const std::lock_guard<std::mutex> lock(m_datareadertcp_sync);

    // make sure we do not have this connection already
    assert (m_datareadertcp_map.find(conn_params) == m_datareadertcp_map.end());

    m_datareadertcp_map.insert(std::make_pair(conn_params, std::make_unique<CDataReaderTCP>(m_attributes, conn_params, m_executor, data_callback)));

    auto erase_data_reader = [this, conn_params]()
    {
      const std::lock_guard<std::mutex> lock(m_datareadertcp_sync);
      m_datareadertcp_map.erase(conn_params);
      };

    // for deletion, we need to be able to erase the connection from the map
    return CTransportLayerInstance::ConnectionToken::Make(erase_data_reader);
  }
}
