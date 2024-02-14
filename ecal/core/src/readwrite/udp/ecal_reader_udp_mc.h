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
 * @brief  udp multicast reader and layer
**/

#pragma once

#include "io/udp/ecal_udp_sample_receiver.h"
#include "readwrite/ecal_reader_layer.h"

#include <cstddef>
#include <map>
#include <memory>
#include <string>

namespace eCAL
{
  ////////////////
  // LAYER
  ////////////////
  class CUDPReaderLayer : public CReaderLayer<CUDPReaderLayer>
  {
  public:
    CUDPReaderLayer();
    ~CUDPReaderLayer() override;

    void Initialize() override;

    void AddSubscription(const std::string& /*host_name_*/, const std::string& topic_name_, const std::string& /*topic_id_*/) override;
    void RemSubscription(const std::string& /*host_name_*/, const std::string& topic_name_, const std::string& /*topic_id_*/) override;

    void SetConnectionParameter(SReaderLayerPar& /*par_*/) override {}

  private:
    bool HasSample(const std::string& sample_name_);
    bool ApplySample(const char* serialized_sample_data_, size_t serialized_sample_size_);

    bool                                   m_started;
    bool                                   m_local_mode;
    std::shared_ptr<UDP::CSampleReceiver>  m_payload_receiver;
    std::map<std::string, int>             m_topic_name_mcast_map;
  };
}
