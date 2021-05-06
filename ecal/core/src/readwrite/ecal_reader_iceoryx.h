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
 * @brief  shared memory (iceoryx) reader
**/

#pragma once

#include "readwrite/ecal_reader_layer.h"

#include <iceoryx_posh/popo/listener.hpp>
#include <iceoryx_posh/popo/untyped_subscriber.hpp>

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace eCAL
{
  // ecal shared memory (Iceoryx) reader
  class CDataReaderSHM
  {
  public:
    CDataReaderSHM();

    bool CreateIceoryxSub(const std::string& topic_name_);
    bool DestroyIceoryxSub();

  private:
    std::shared_ptr<iox::popo::UntypedSubscriber> m_subscriber;
    iox::popo::Listener                           m_listener;
    static void onSampleReceivedCallback(iox::popo::UntypedSubscriber* subscriber_, CDataReaderSHM* self);

    std::string m_topic_name;
  };

  // ecal Iceoryx data layer
  class CSHMLayer : public CReaderLayer<CSHMLayer>
  {
  public:
    CSHMLayer() {}

    void Initialize()
    {
    }

    void AddSubscription(std::string& topic_name_, std::string& topic_id_, QOS::SReaderQOS /*qos_*/)
    {
      std::lock_guard<std::mutex> lock(m_datareadershm_sync);
      if(m_datareadershm_map.find(topic_id_) != m_datareadershm_map.end()) return;

      std::shared_ptr<CDataReaderSHM> reader = std::make_shared<CDataReaderSHM>();
      reader->CreateIceoryxSub(topic_name_);

      m_datareadershm_map.insert(std::pair<std::string, std::shared_ptr<CDataReaderSHM>>(topic_id_, reader));
    }

    void RemSubscription(std::string& /*topic_name_*/, std::string& topic_id_)
    {
      std::lock_guard<std::mutex> lock(m_datareadershm_sync);
      DataReaderSHMMapT::iterator iter = m_datareadershm_map.find(topic_id_);
      if(iter == m_datareadershm_map.end()) return;

      auto reader = iter->second;
      reader->DestroyIceoryxSub();

      m_datareadershm_map.erase(iter);
    }

    void UpdateParameter(SReaderLayerPar& /*par_*/)
    {
    }

  private:
    typedef std::unordered_map<std::string, std::shared_ptr<CDataReaderSHM>> DataReaderSHMMapT;
    std::mutex        m_datareadershm_sync;
    DataReaderSHMMapT m_datareadershm_map;
  };
}
