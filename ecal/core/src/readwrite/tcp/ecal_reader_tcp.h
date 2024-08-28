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

#pragma once

#include "readwrite/ecal_reader_layer.h"

#include <tcp_pubsub/executor.h>
#include <tcp_pubsub/subscriber.h>

#include "serialization/ecal_struct_sample_payload.h"

#include <atomic>
#include <mutex>
#include <unordered_map>

namespace eCAL
{
  ////////////////
  // READER
  ////////////////
  class CDataReaderTCP
  {
  public:
    CDataReaderTCP();

    bool Create(std::shared_ptr<tcp_pubsub::Executor>& executor_);
    bool Destroy();

    bool AddConnectionIfNecessary(const std::string& host_name_, uint16_t port_);

  private:
    void OnTcpMessage(const tcp_pubsub::CallbackData& callback_data);

    Payload::Sample                         m_ecal_header;

    std::shared_ptr<tcp_pubsub::Subscriber> m_subscriber;
    bool                                    m_callback_active;
  };

  ////////////////
  // LAYER
  ////////////////
  class CTCPReaderLayer : public CReaderLayer<CTCPReaderLayer>
  {
  public:
    CTCPReaderLayer();

    void Initialize() override;

    void AddSubscription(const std::string& host_name_, const std::string& topic_name_, const std::string& topic_id_) override;
    void RemSubscription(const std::string& host_name_, const std::string& topic_name_, const std::string& topic_id_) override;

    void SetConnectionParameter(SReaderLayerPar& /*par_*/) override;

  private:
    std::atomic<bool> m_initialized;
    std::shared_ptr<tcp_pubsub::Executor> m_executor;

    using DataReaderTCPMapT = std::unordered_map<std::string, std::shared_ptr<CDataReaderTCP>>;
    std::mutex        m_datareadertcp_sync;
    DataReaderTCPMapT m_datareadertcp_map;
  };
}
