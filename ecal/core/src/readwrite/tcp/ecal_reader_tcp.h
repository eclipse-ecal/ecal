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
 * @brief  tcp reader and layer
**/

#pragma once

#include "readwrite/ecal_reader_layer.h"

#include <map>
#include <mutex>

#include <tcp_pubsub/executor.h>
#include <tcp_pubsub/subscriber.h>

#include "config/attributes/data_reader_tcp_attributes.h"
#include "serialization/ecal_struct_sample_payload.h"

namespace eCAL
{
  ////////////////
  // READER
  ////////////////
  struct STCPConnectionParameters
  {
    std::string host_name;
    uint16_t    port{ 0 };

    bool operator<(const STCPConnectionParameters& other) const
    {
      return std::tie(port, host_name) < std::tie(other.port, other.host_name);
    }

    bool operator==(const STCPConnectionParameters& other) const
    {
      return std::tie(port, host_name) == std::tie(other.port, other.host_name);
    }
  };

  class CDataReaderTCP
  {
  public:
    using DataCallback = std::function<void(const SReceiveCallbackData& /*data_*/)>;

    CDataReaderTCP(const eCAL::eCALReader::TCP::SAttributes& attr_, 
      const STCPConnectionParameters& conn_params_,
      std::shared_ptr<tcp_pubsub::Executor>& executor_,
      DataCallback on_data_);

  private:
    void OnTcpMessage(const tcp_pubsub::CallbackData& callback_data);
    Payload::Sample                         m_ecal_header;

    eCAL::eCALReader::TCP::SAttributes      m_attributes;
    DataCallback                            m_on_data;

    std::unique_ptr<tcp_pubsub::Subscriber> m_subscriber;
  };

  ////////////////
  // LAYER
  ////////////////
  class CTCPReaderLayer : CTransportLayerInstance
  {
  public:
    CTCPReaderLayer(const eCAL::eCALReader::TCP::SAttributes& attr_);
    ~CTCPReaderLayer() override = default;

    bool AcceptsConnection(const PublisherConnectionParameters& publisher, const SubscriberConnectionParameters& subscriber) const override;
    CTransportLayerInstance::ConnectionToken AddConnection(const PublisherConnectionParameters& publisher, const ReceiveCallbackT& on_data, const ConnectionChangeCallback& on_connection_changed) override;

  private:
    // global, per process TCP attributes
    eCAL::eCALReader::TCP::SAttributes m_attributes;

    std::shared_ptr<tcp_pubsub::Executor> m_executor;

    using DataReaderTCPMapT = std::map<STCPConnectionParameters, std::unique_ptr<CDataReaderTCP>>;
    std::mutex        m_datareadertcp_sync;
    DataReaderTCPMapT m_datareadertcp_map;
  };
}
