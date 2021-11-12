/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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
 * @brief  tcp reader
**/

#include "ecal_def.h"
#include "ecal_config_hlp.h"
#include "ecal_global_accessors.h"
#include "pubsub/ecal_subgate.h"

#include "readwrite/ecal_writer_base.h"
#include "readwrite/ecal_reader_tcp.h"

#include "ecal_utils/portable_endian.h"

namespace eCAL
{
  template<> std::shared_ptr<CTCPReaderLayer> CReaderLayer<CTCPReaderLayer>::layer(nullptr);

  //////////////////////////////////////////////////////////////////
  // CDataReaderTCP
  //////////////////////////////////////////////////////////////////
  CDataReaderTCP::CDataReaderTCP() = default;

  CTCPReaderLayer::CTCPReaderLayer()
  {
  }

  void CTCPReaderLayer::Initialize()
  {
    m_executor = std::make_shared<tcpub::Executor>(4);
  }

  void CTCPReaderLayer::AddSubscription(std::string& host_name_, std::string& /*topic_name_*/, std::string& topic_id_, QOS::SReaderQOS /*qos_*/)
  {
    std::string map_key(host_name_ + topic_id_);

    std::lock_guard<std::mutex> lock(m_datareadertcp_sync);
    if (m_datareadertcp_map.find(map_key) != m_datareadertcp_map.end()) return;

    std::shared_ptr<CDataReaderTCP> reader = std::make_shared<CDataReaderTCP>();
    reader->Create(m_executor);

    m_datareadertcp_map.insert(std::pair<std::string, std::shared_ptr<CDataReaderTCP>>(map_key, reader));
  }

  void CTCPReaderLayer::RemSubscription(std::string& host_name_, std::string& /*topic_name_*/, std::string& topic_id_)
  {
    std::string map_key(host_name_ + topic_id_);

    std::lock_guard<std::mutex> lock(m_datareadertcp_sync);
    DataReaderTCPMapT::iterator iter = m_datareadertcp_map.find(map_key);
    if (iter == m_datareadertcp_map.end()) return;

    auto reader = iter->second;
    reader->Destroy();

    m_datareadertcp_map.erase(iter);
  }

  void CTCPReaderLayer::SetConnectionParameter(SReaderLayerPar& par_)
  {
    eCAL::pb::ConnnectionPar connection_par;
    if (connection_par.ParseFromString(par_.parameter))
    {
      //////////////////////////////////
      // get parameter from a new writer
      //////////////////////////////////
      // host name
      auto host_name = par_.host_name;
      // topic id
      auto topic_id  = par_.topic_id;
      // port
      auto port = connection_par.layer_par_tcp().port();

      std::string map_key(host_name + topic_id);

      std::lock_guard<std::mutex> lock(m_datareadertcp_sync);
      DataReaderTCPMapT::iterator iter = m_datareadertcp_map.find(map_key);
      if (iter == m_datareadertcp_map.end()) return;

      auto reader = iter->second;
      reader->SetConnection(host_name, static_cast<uint16_t>(port));
    }
    else
    {
      std::cout << "FATAL ERROR: Could not parse layer connection parameter ! Did you mix up different eCAL versions on the same host ?" << std::endl;
      return;
    }
  }

  bool CDataReaderTCP::Create(std::shared_ptr<tcpub::Executor>& executor_)
  {
    // create tcp subscriber
    m_subscriber = std::make_shared<tcpub::Subscriber>(executor_);
    m_subscriber->setCallback(std::bind(&CDataReaderTCP::OnTcpMessage, this, std::placeholders::_1));
    return true;
  }

  bool CDataReaderTCP::Destroy()
  {
    if (!m_subscriber) return false;
    m_subscriber = nullptr;
    return true;
  }

  bool CDataReaderTCP::SetConnection(const std::string& host_name_, uint16_t port_)
  {
    if (!m_subscriber) return false;
    if (port_ == 0)    return false;

    // check for new session
    bool new_session(true);
    auto sessions = m_subscriber->getSessions();
    for (auto session : sessions)
    {
      auto address = session->getAddress();
      auto port    = session->getPort();
      if ((address == host_name_) && (port == port_))
      {
        new_session = false;
        break;
      }
    }

    // add new session
    if (new_session)
    {
      m_subscriber->addSession(host_name_, port_);
    }

    return true;
  }

  void CDataReaderTCP::OnTcpMessage(const tcpub::CallbackData& data_)
  {
    // extract header size
    const size_t ecal_magic(4 * sizeof(char));
    //                           ECAL        +  payload size field
    const size_t header_length = ecal_magic  +  sizeof(uint16_t);
    uint16_t     header_size   = le16toh(*reinterpret_cast<uint16_t*>(data_.buffer_->data() + ecal_magic));

    // extract header
    const char* header_payload = data_.buffer_->data() + header_length;
    // extract data payload
    const char* data_payload   = header_payload + header_size;

    // parse header
    if (m_ecal_header.ParseFromArray(header_payload, static_cast<int>(header_size)))
    {
      if (g_subgate())
      {
        // apply sample
        g_subgate()->ApplySample(
          m_ecal_header.topic().tname(),
          m_ecal_header.topic().tid(),
          data_payload,
          static_cast<size_t>(m_ecal_header.content().size()),
          m_ecal_header.content().id(),
          m_ecal_header.content().clock(),
          m_ecal_header.content().time(),
          m_ecal_header.content().hash(),
          eCAL::pb::tl_ecal_tcp);
      }
    }
  };
}
