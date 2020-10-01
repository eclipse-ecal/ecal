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

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace eCAL
{
  // ecal Iceoryx reader
  class CDataReaderSHM
  {
  public:
    CDataReaderSHM();

    bool CreateIceoryxSub(const std::string& topic_name_);
    bool DestroyIceoryxSub(const std::string& topic_name_);

  private:
    std::shared_ptr<iox::popo::Subscriber> m_subscriber;
    void receiveHandler();

    std::string m_topic_name;
  };

  // ecal Iceoryx data layer
  class CSHMLayer : public CReaderLayer<CSHMLayer>
  {
  public:
    CSHMLayer() {}

    void InitializeLayer()
    {
    }

    void StartLayer(std::string& topic_name_, QOS::SReaderQOS /*qos_*/)
    {
      std::lock_guard<std::mutex> lock(m_topic_name_datareadershm_sync);
      if(m_topic_name_datareadershm_map.find(topic_name_) != m_topic_name_datareadershm_map.end()) return;

      std::shared_ptr<CDataReaderSHM> reader = std::make_shared<CDataReaderSHM>();
      reader->CreateIceoryxSub(topic_name_);

      m_topic_name_datareadershm_map.insert(std::pair<std::string, std::shared_ptr<CDataReaderSHM>>(topic_name_, reader));
    }

    void StopLayer(std::string& topic_name_)
    {
      std::lock_guard<std::mutex> lock(m_topic_name_datareadershm_sync);
      TopicNameDataReaderSHMMapT::iterator iter = m_topic_name_datareadershm_map.find(topic_name_);
      if(iter == m_topic_name_datareadershm_map.end()) return;

      auto reader = iter->second;
      reader->DestroyIceoryxSub(topic_name_);

      m_topic_name_datareadershm_map.erase(iter);
    }

    void ApplyLayerParameter(SReaderLayerPar& /*par_*/)
    {
    }

  private:
    typedef std::unordered_map<std::string, std::shared_ptr<CDataReaderSHM>> TopicNameDataReaderSHMMapT;
    std::mutex                 m_topic_name_datareadershm_sync;
    TopicNameDataReaderSHMMapT m_topic_name_datareadershm_map;
  };
}
