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

#pragma once

#include "readwrite/ecal_reader_layer.h"

#include <tcp_pubsub/executor.h>
#include <tcp_pubsub/subscriber.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include "ecal/pb/ecal.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace eCAL
{
  // ecal tcp reader
  class CDataReaderTCP
  {
  public:
    CDataReaderTCP();

    bool Create(std::shared_ptr<tcp_pubsub::Executor>& executor_);
    bool Destroy();

    bool SetConnection(const std::string& host_name_, uint16_t port_);

  private:
    void OnTcpMessage(const tcp_pubsub::CallbackData& callback_data);
    std::shared_ptr<tcp_pubsub::Subscriber> m_subscriber;
    eCAL::pb::Sample                   m_ecal_header;
  };

  // ecal tcp reader data layer
  class CTCPReaderLayer : public CReaderLayer<CTCPReaderLayer>
  {
  public:
    CTCPReaderLayer();

    void Initialize();

    void AddSubscription(const std::string& host_name_, const std::string& topic_name_, const std::string& topic_id_, QOS::SReaderQOS qos_);
    void RemSubscription(const std::string& host_name_, const std::string& topic_name_, const std::string& topic_id_);

    void SetConnectionParameter(SReaderLayerPar& /*par_*/);

  private:
    std::shared_ptr<tcp_pubsub::Executor> m_executor;

    typedef std::unordered_map<std::string, std::shared_ptr<CDataReaderTCP>> DataReaderTCPMapT;
    std::mutex        m_datareadertcp_sync;
    DataReaderTCPMapT m_datareadertcp_map;
  };
}
