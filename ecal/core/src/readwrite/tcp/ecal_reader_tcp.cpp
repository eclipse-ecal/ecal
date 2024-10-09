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
 * @brief  tcp reader and layer
**/

#include <ecal/ecal_config.h>

#include "ecal_global_accessors.h"
#include "ecal_reader_tcp.h"
#include "ecal_tcp_pubsub_logger.h"

#include "pubsub/ecal_subgate.h"

#include "ecal_utils/portable_endian.h"

namespace eCAL
{
  ////////////////
  // READER
  ////////////////
  CDataReaderTCP::CDataReaderTCP() : m_callback_active(false) {}

  bool CDataReaderTCP::Create(std::shared_ptr<tcp_pubsub::Executor>& executor_)
  {
    // create tcp subscriber
    m_subscriber = std::make_shared<tcp_pubsub::Subscriber>(executor_);
    return true;
  }

  bool CDataReaderTCP::Destroy()
  {
    if (!m_subscriber) return false;
    m_subscriber      = nullptr;
    m_callback_active = false;
    return true;
  }

  bool CDataReaderTCP::AddConnectionIfNecessary(const std::string& host_name_, uint16_t port_)
  {
    if (!m_subscriber) return false;
    if (port_ == 0)    return false;

    // check for new session
    bool new_session(true);
    auto sessions = m_subscriber->getSessions();
    for (const auto& session : sessions)
    {
      auto address = session->getAddress();
      auto port    = session->getPort();
      if ((address == host_name_) && (port == port_))
      {
        new_session = false;
        break;
      }
    }

    // add new session and activate callback if we add the first session
    if (new_session)
    {
      m_subscriber->addSession(host_name_, port_, Config::GetTcpPubsubMaxReconnectionAttemps());
      if (!m_callback_active)
      {
        m_subscriber->setCallback(std::bind(&CDataReaderTCP::OnTcpMessage, this, std::placeholders::_1));
        m_callback_active = true;
      }
    }

    return true;
  }

  void CDataReaderTCP::OnTcpMessage(const tcp_pubsub::CallbackData& data_)
  {
    // extract header size
    const size_t ecal_magic(4 * sizeof(char));
    //                             ECAL       +  header size field
    const size_t   header_length = ecal_magic + sizeof(uint16_t);
    const uint16_t header_size   = le16toh(*reinterpret_cast<uint16_t*>(data_.buffer_->data() + ecal_magic));

    // extract header
    const char* header_payload = data_.buffer_->data() + header_length;
    // extract data payload
    const char* data_payload   = header_payload + header_size;

    // parse header
    if (DeserializeFromBuffer(header_payload, header_size, m_ecal_header))
    {
      if (g_subgate() != nullptr)
      {
        // use this intermediate variables as optimization
        const auto& ecal_header_topic   = m_ecal_header.topic;
        const auto& ecal_header_content = m_ecal_header.content;
        // apply sample
        g_subgate()->ApplySample(
          ecal_header_topic.tname,
          ecal_header_topic.tid,
          data_payload,
          static_cast<size_t>(ecal_header_content.size),
          ecal_header_content.id,
          ecal_header_content.clock,
          ecal_header_content.time,
          ecal_header_content.hash,
          tl_ecal_tcp);
      }
    }
  }
  
  ////////////////
  // LAYER
  ////////////////
  CTCPReaderLayer::CTCPReaderLayer() : m_initialized(false) {}

  void CTCPReaderLayer::Initialize()
  {
    if (m_initialized) return;
    m_initialized = true;

    const tcp_pubsub::logger::logger_t tcp_pubsub_logger = std::bind(TcpPubsubLogger, std::placeholders::_1, std::placeholders::_2);
    m_executor = std::make_shared<tcp_pubsub::Executor>(Config::GetTcpPubsubReaderThreadpoolSize(), tcp_pubsub_logger);
  }

  void CTCPReaderLayer::AddSubscription(const std::string& /*host_name_*/, const std::string& topic_name_, const std::string& /*topic_id_*/)
  {
    const std::string& map_key(topic_name_);

    const std::lock_guard<std::mutex> lock(m_datareadertcp_sync);
    if (m_datareadertcp_map.find(map_key) != m_datareadertcp_map.end()) return;

    const std::shared_ptr<CDataReaderTCP> reader = std::make_shared<CDataReaderTCP>();
    reader->Create(m_executor);

    m_datareadertcp_map.insert(std::pair<std::string, std::shared_ptr<CDataReaderTCP>>(map_key, reader));
  }

  void CTCPReaderLayer::RemSubscription(const std::string& /*host_name_*/, const std::string& topic_name_, const std::string& /*topic_id_*/)
  {
    const std::string& map_key(topic_name_);

    const std::lock_guard<std::mutex> lock(m_datareadertcp_sync);
    const DataReaderTCPMapT::iterator iter = m_datareadertcp_map.find(map_key);
    if (iter == m_datareadertcp_map.end()) return;

    auto reader = iter->second;
    reader->Destroy();

    m_datareadertcp_map.erase(iter);
  }

  void CTCPReaderLayer::SetConnectionParameter(SReaderLayerPar& par_)
  {
    //////////////////////////////////
    // get parameter from a new writer
    //////////////////////////////////
    const auto& remote_hostname = par_.host_name;
    auto        remote_port     = par_.parameter.layer_par_tcp.port;

    const std::string map_key(par_.topic_name);

    const std::lock_guard<std::mutex> lock(m_datareadertcp_sync);
    const DataReaderTCPMapT::iterator iter = m_datareadertcp_map.find(map_key);
    if (iter == m_datareadertcp_map.end()) return;

    auto& reader = iter->second;
    reader->AddConnectionIfNecessary(remote_hostname, static_cast<uint16_t>(remote_port));
  }
}
