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

#include "ecal_def.h"
#include "ecal_config_hlp.h"
#include "ecal_global_accessors.h"
#include "pubsub/ecal_subgate.h"

#include "ecal/ecal_process.h"
#include "readwrite/ecal_writer_base.h"
#include "readwrite/ecal_reader_iceoryx.h"

#include <iceoryx_posh/runtime/posh_runtime.hpp>

namespace eCAL
{
  template<> std::shared_ptr<CSHMLayer> CReaderLayer<CSHMLayer>::layer(nullptr);

  //////////////////////////////////////////////////////////////////
  // CDataReaderIceoryx
  //////////////////////////////////////////////////////////////////
  CDataReaderSHM::CDataReaderSHM()
  {
  }

  bool CDataReaderSHM::CreateIceoryxSub(const std::string& topic_name_)
  {
    m_topic_name = topic_name_;

    // create the runtime for registering with the RouDi daemon
    iox::runtime::PoshRuntime::getInstance(std::string("/") + eCAL::Process::GetUnitName() + std::string("_") + std::to_string(eCAL::Process::GetProcessID()));

    // create subscriber
    m_subscriber = std::shared_ptr<iox::popo::Subscriber>(new iox::popo::Subscriber({eCALPAR(ICEORYX, SERVICE), eCALPAR(ICEORYX, INSTANCE), topic_name_}));
    m_subscriber->setReceiveHandler(std::bind(&CDataReaderSHM::receiveHandler, this));
    m_subscriber->subscribe();

    return true;
  }

  bool CDataReaderSHM::DestroyIceoryxSub(const std::string& /*topic_name_*/)
  {
    if(!m_subscriber) return false;

    m_subscriber->unsubscribe();
    m_subscriber->unsetReceiveHandler();
    m_subscriber = nullptr;

    return true;
  }

  void CDataReaderSHM::receiveHandler()
  {
    const iox::mepoo::ChunkHeader* ch(nullptr);

    // get all the chunks the FiFo holds. Maybe there are several ones if the publisher produces faster than the subscriber can process
    while (m_subscriber->getChunk(&ch))
    {
      // extract data header
      const size_t header_size = sizeof(CDataWriterBase::SWriterData);
      const CDataWriterBase::SWriterData* data_header  = static_cast<CDataWriterBase::SWriterData*>(ch->payload());
      const char*                         data_payload = static_cast<char*>(ch->payload()) + header_size;
      if(ch->m_info.m_payloadSize == header_size + data_header->len)
      {
        // apply data to subscriber gate
        if (g_subgate()) g_subgate()->ApplySample(m_topic_name, /*topic_id_*/ "", data_payload, data_header->len, data_header->id, data_header->clock, data_header->time, data_header->hash, eCAL::pb::eTLayerType::tl_ecal_shm);
      }
      // release the chunk
      m_subscriber->releaseChunk(ch);
    }
  }
}
