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
 * @brief  eCAL iceoryx reader
**/

#pragma once

#include "readwrite/ecal_reader_layer.h"

#include <iceoryx_posh/popo/subscriber.hpp>

namespace eCAL
{
  // ecal Iceoryx reader
  class CDataReaderIceoryx
  {
  public:
    CDataReaderIceoryx();

    bool CreateIceoryxSub(const std::string& topic_name_);
    bool DestroyIceoryxSub(const std::string& topic_name_);

  private:
    std::shared_ptr<iox::popo::Subscriber> m_subscriber;
    void receiveHandler();

    std::string m_topic_name;
  };

  // ecal Iceoryx data layer
  class CIceoryxLayer : public CReaderLayer<CIceoryxLayer>
  {
  public:
    CIceoryxLayer() {}

    void InitializeLayer()
    {
    }

    void StartLayer(std::string& topic_name_, QOS::SReaderQOS /*qos_*/)
    {
      reader.CreateIceoryxSub(topic_name_);
    }

    void StopLayer(std::string& topic_name_)
    {
      reader.DestroyIceoryxSub(topic_name_);
    }

    void ApplyLayerParameter(SReaderLayerPar& /*par_*/)
    {
    }

  private:
    CDataReaderIceoryx reader;
  };
}
