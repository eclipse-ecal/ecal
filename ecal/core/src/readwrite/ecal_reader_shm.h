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
 * @brief  shared memory layer
**/

#pragma once

#include "ecal_def.h"
#include "readwrite/ecal_reader_layer.h"

#include <memory>

namespace eCAL
{
  ////////////////
  // LAYER
  ////////////////
  class CSHMReaderLayer : public CReaderLayer<CSHMReaderLayer>
  {
  public:
    CSHMReaderLayer() = default;
    ~CSHMReaderLayer() override = default;

    void Initialize() override {}
    void AddSubscription(const std::string& /*host_name_*/, const std::string& /*topic_name_*/, const std::string& /*topic_id_*/, QOS::SReaderQOS /*qos_*/) override {}
    void RemSubscription(const std::string& /*host_name_*/, const std::string& /*topic_name_*/, const std::string& /*topic_id_*/) override {}

    void SetConnectionParameter(SReaderLayerPar& par_) override;

  private:
    size_t OnNewShmFileContent(const std::string& topic_name_, const std::string& topic_id_, const char* buf_, size_t len_, long long id_, long long clock_, long long time_, size_t hash_);
  };
}
