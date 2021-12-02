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
 * @brief  memory file data reader
**/

#pragma once

#include "ecal_def.h"
#include "readwrite/ecal_reader_layer.h"

#include <memory>

namespace eCAL
{
  // ecal shared memory reader
  class CDataReaderSHM
  {
  public:
    CDataReaderSHM() = default;
    ~CDataReaderSHM() = default;
  };

  // ecal shared memory layer
  class CSHMLayer : public CReaderLayer<CSHMLayer>
  {
  public:
    CSHMLayer() {};
    ~CSHMLayer()
    {
    };

    void Initialize()
    {
    }

    void AddSubscription(const std::string& /*host_name_*/, const std::string& /*topic_name_*/, const std::string& /*topic_id_*/, QOS::SReaderQOS /*qos_*/)
    {
    }

    void RemSubscription(const std::string& /*host_name_*/, const std::string& /*topic_name_*/, const std::string& /*topic_id_*/)
    {
    }

    void SetConnectionParameter(SReaderLayerPar& par_);
  };
}
