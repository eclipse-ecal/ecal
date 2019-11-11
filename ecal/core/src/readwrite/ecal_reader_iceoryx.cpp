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

#include "ecal_global_accessors.h"
#include "pubsub/ecal_subgate.h"

#include "ecal/ecal_process.h"
#include "readwrite/ecal_reader_iceoryx.h"

#include <iceoryx_posh/runtime/posh_runtime.hpp>

namespace eCAL
{
  template<> std::shared_ptr<CIceoryxLayer> CReaderLayer<CIceoryxLayer>::layer(nullptr);

  //////////////////////////////////////////////////////////////////
  // CDataReaderIceoryx
  //////////////////////////////////////////////////////////////////
  CDataReaderIceoryx::CDataReaderIceoryx()
  {
  }

  bool CDataReaderIceoryx::CreateIceoryxSub(const std::string& topic_name_)
  {
    m_topic_name = topic_name_;

    // create the runtime for registering with the RouDi daemon
    iox::runtime::PoshRuntime::getInstance(std::string("/") + eCAL::Process::GetUnitName());

    // create subscriber
    m_subscriber = std::shared_ptr<iox::popo::Subscriber>(new iox::popo::Subscriber({"eCAL", "", topic_name_}));
    m_subscriber->setReceiveHandler(std::bind(&CDataReaderIceoryx::receiveHandler, this));
    m_subscriber->subscribe();

    return true;
  }

  bool CDataReaderIceoryx::DestroyIceoryxSub(const std::string& /*topic_name_*/)
  {
    if(!m_subscriber) return false;

    m_subscriber->unsubscribe();
    m_subscriber->unsetReceiveHandler();
    m_subscriber = nullptr;

    return true;
  }

  void CDataReaderIceoryx::receiveHandler()
  {
    const iox::mepoo::ChunkInfo* ci(nullptr);

    // get all the chunks the FiFo holds. Maybe there are several ones if the publisher produces faster than the subscriber can process
    while (m_subscriber->getChunkWithInfo(&ci))
    {
      // apply data to subscriber gate
      if (g_subgate()) g_subgate()->ApplySample(m_topic_name, "", static_cast<const char*>(ci->m_payload), ci->m_payloadSize, /* msg_id = */ 0, ci->m_sequenceNumber, /*ci->m_txTimestamp*/ 0, /* msg_hash = */ 0, eCAL::pb::eTLayerType::tl_iceoryx);

      // release the chunk
      m_subscriber->releaseChunkWithInfo(ci);
    }
  }
}
