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

namespace eCAL
{
  // ecal tcp reader
  class CDataReaderTCP
  {
  public:
    CDataReaderTCP();

    bool Create(const std::string& topic_name_);
    bool Destroy();

  private:
    std::string m_topic_name;
  };

  // ecal tcp data layer
  class CTCPLayer : public CReaderLayer<CTCPLayer>
  {
  public:
    CTCPLayer() {}

    void Initialize()
    {
      int foo(0);
    }

    void AddSubscription(std::string& topic_name_, std::string& topic_id_, QOS::SReaderQOS /*qos_*/)
    {
    }

    void RemSubscription(std::string& /*topic_name_*/, std::string& topic_id_)
    {
    }

    void SetConnectionParameter(SReaderLayerPar& /*par_*/)
    {
    }

  private:
  };
}
