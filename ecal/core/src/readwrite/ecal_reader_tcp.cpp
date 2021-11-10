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

  void CTCPReaderLayer::AddSubscription(std::string& /*topic_name_*/, std::string& topic_id_, QOS::SReaderQOS /*qos_*/)
  {
    std::lock_guard<std::mutex> lock(m_datareadershm_sync);
    if (m_datareadershm_map.find(topic_id_) != m_datareadershm_map.end()) return;

    std::shared_ptr<CDataReaderTCP> reader = std::make_shared<CDataReaderTCP>();
    reader->Create(m_executor);

    m_datareadershm_map.insert(std::pair<std::string, std::shared_ptr<CDataReaderTCP>>(topic_id_, reader));
  }

  void CTCPReaderLayer::RemSubscription(std::string& /*topic_name_*/, std::string& topic_id_)
  {
    std::lock_guard<std::mutex> lock(m_datareadershm_sync);
    DataReaderSHMMapT::iterator iter = m_datareadershm_map.find(topic_id_);
    if (iter == m_datareadershm_map.end()) return;

    auto reader = iter->second;
    reader->Destroy();

    m_datareadershm_map.erase(iter);
  }

  void CTCPReaderLayer::SetConnectionParameter(SReaderLayerPar& par_)
  {
    eCAL::pb::ConnnectionPar connection_par;
    if (connection_par.ParseFromString(par_.parameter))
    {
      //////////////////////////////////
      // get parameter from a new writer
      //////////////////////////////////
      // topic name
      auto topic_name = par_.topic_name;
      // topic id
      auto topic_id   = par_.topic_id;
      // host name
      auto host_name  = par_.host_name;
      // port
      auto port = connection_par.layer_par_tcp().port();

      std::lock_guard<std::mutex> lock(m_datareadershm_sync);
      DataReaderSHMMapT::iterator iter = m_datareadershm_map.find(topic_id);
      if (iter == m_datareadershm_map.end()) return;

      auto reader = iter->second;
      reader->SetPortNumber(static_cast<uint16_t>(port));
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

  bool CDataReaderTCP::SetPortNumber(uint16_t port_)
  {
    if (m_port != 0)   return false;
    if (!m_subscriber) return false;
    if (port_ == 0)    return false;
    m_port = port_;
    m_subscriber->addSession("127.0.0.1", m_port);
    return true;
  }

  void CDataReaderTCP::OnTcpMessage(const tcpub::CallbackData& data_)
  {
    m_ecal_sample.Clear();
    if (m_ecal_sample.ParseFromArray(data_.buffer_->data(), data_.buffer_->size()))
    {
      if (g_subgate()) g_subgate()->ApplySample(m_ecal_sample, eCAL::pb::tl_ecal_tcp);
    }
  };
}
