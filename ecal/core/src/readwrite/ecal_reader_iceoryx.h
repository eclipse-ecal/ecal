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
 * @brief  shared memory reader and layer (iceoryx)
**/

#pragma once

#include "readwrite/ecal_reader_layer.h"

#include <iceoryx_posh/popo/listener.hpp>
#include <iceoryx_posh/popo/untyped_subscriber.hpp>

#include <memory>
#include <mutex>
#include <regex>
#include <string>
#include <unordered_map>

namespace eCAL
{
  ////////////////
  // READER
  ////////////////
  class CDataReaderSHM
  {
  public:
    CDataReaderSHM();

    bool Create(const std::string& topic_name_);
    bool Destroy();

  private:
    std::shared_ptr<iox::popo::UntypedSubscriber> m_subscriber;
    std::shared_ptr<iox::popo::Listener>          m_listener;
    static void onSampleReceivedCallback(iox::popo::UntypedSubscriber* subscriber_, CDataReaderSHM* self);

    std::string m_topic_name;
  };

  ////////////////
  // LAYER
  ////////////////
  class CSHMReaderLayer : public CReaderLayer<CSHMReaderLayer>
  {
  public:
    CSHMReaderLayer() {}

    void Initialize();
    void AddSubscription(const std::string& /*host_name_*/, const std::string& topic_name_, const std::string& topic_id_, QOS::SReaderQOS /*qos_*/);
    void RemSubscription(const std::string& /*host_name_*/, const std::string& /*topic_name_*/, const std::string& topic_id_);

    void SetConnectionParameter(SReaderLayerPar& /*par_*/) {}

  private:
    typedef std::unordered_map<std::string, std::shared_ptr<CDataReaderSHM>> DataReaderSHMMapT;
    std::mutex        m_datareadershm_sync;
    DataReaderSHMMapT m_datareadershm_map;
  };
}
