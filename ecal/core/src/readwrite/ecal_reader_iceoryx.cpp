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

#include "ecal_def.h"
#include "ecal_config_reader_hlp.h"
#include "ecal_global_accessors.h"
#include "pubsub/ecal_subgate.h"

#include "ecal/ecal_process.h"
#include "readwrite/ecal_writer_data.h"
#include "readwrite/ecal_reader_iceoryx.h"

#include <iceoryx_posh/runtime/posh_runtime.hpp>

namespace eCAL
{
  ////////////////
  // READER
  ////////////////
  CDataReaderSHM::CDataReaderSHM() = default;

  bool CDataReaderSHM::Create(const std::string& topic_name_)
  {
    // store topic name
    m_topic_name = topic_name_;

    // subscriber description
    const iox::capro::IdString_t service  (iox::cxx::TruncateToCapacity, eCALPAR(ICEORYX, SERVICE));
    const iox::capro::IdString_t instance (iox::cxx::TruncateToCapacity, eCALPAR(ICEORYX, INSTANCE));
    const iox::capro::IdString_t event    (iox::cxx::TruncateToCapacity, topic_name_);
    const iox::capro::ServiceDescription servicedesc(service, instance, event);

    // create subscriber && listener
    m_subscriber = std::make_shared<iox::popo::UntypedSubscriber>(servicedesc);
    m_listener   = std::make_shared<iox::popo::Listener>();

    // attach DATA_RECEIVED event
    m_listener
        ->attachEvent(*m_subscriber,
                     iox::popo::SubscriberEvent::DATA_RECEIVED,
                     iox::popo::createNotificationCallback(onSampleReceivedCallback, *this))
        .or_else([](auto) {
          Logging::Log(log_level_fatal, "CDataWriterSHM::CreateIceoryxSub(): Unable to attach subscriber to listener");
        });

    return true;
  }

  bool CDataReaderSHM::Destroy()
  {
    if(!m_subscriber) return false;

    // detach event and destroy subscriber and listener
    m_listener->detachEvent(*m_subscriber, iox::popo::SubscriberEvent::DATA_RECEIVED);
    m_subscriber = nullptr;
    m_listener   = nullptr;
    
    return true;
  }

  void CDataReaderSHM::onSampleReceivedCallback(iox::popo::UntypedSubscriber* subscriber_, CDataReaderSHM* self)
  {
    subscriber_->take().and_then([subscriber_, self](auto& userPayload) {
      // extract data header
      auto data_header  = static_cast<const SWriterData*>(iox::mepoo::ChunkHeader::fromUserPayload(userPayload)->userHeader());
      // extract data
      auto data_payload = static_cast<const char*>(userPayload);
      // apply data to subscriber gate
      if (g_subgate()) g_subgate()->ApplySample(self->m_topic_name, /*topic_id_*/ "", data_payload, data_header->len, data_header->id, data_header->clock, data_header->time, data_header->hash, eCAL::pb::eTLayerType::tl_ecal_shm);
      // release payload
      subscriber_->release(userPayload);
    });
  }
  
  ////////////////
  // LAYER
  ////////////////
  void CSHMReaderLayer::Initialize()
  {
    // create the runtime for registering with the RouDi daemon
    std::string runtime_name = eCAL::Process::GetUnitName() + std::string("_") + std::to_string(eCAL::Process::GetProcessID());
    // replace whitespace characters
    std::regex re("[ \t\r\n\f]");
    runtime_name = std::regex_replace(runtime_name, re, "_");
    // initialize runtime
    const iox::capro::IdString_t runtime(iox::cxx::TruncateToCapacity, runtime_name);
    iox::runtime::PoshRuntime::initRuntime(runtime);
  }

  void CSHMReaderLayer::AddSubscription(const std::string& /*host_name_*/, const std::string& topic_name_, const std::string& topic_id_, QOS::SReaderQOS /*qos_*/)
  {
    std::lock_guard<std::mutex> lock(m_datareadershm_sync);
    if(m_datareadershm_map.find(topic_id_) != m_datareadershm_map.end()) return;

    std::shared_ptr<CDataReaderSHM> reader = std::make_shared<CDataReaderSHM>();
    reader->Create(topic_name_);

    m_datareadershm_map.insert(std::pair<std::string, std::shared_ptr<CDataReaderSHM>>(topic_id_, reader));
  }

  void CSHMReaderLayer::RemSubscription(const std::string& /*host_name_*/, const std::string& /*topic_name_*/, const std::string& topic_id_)
  {
    std::lock_guard<std::mutex> lock(m_datareadershm_sync);
    DataReaderSHMMapT::iterator iter = m_datareadershm_map.find(topic_id_);
    if(iter == m_datareadershm_map.end()) return;

    auto reader = iter->second;
    reader->Destroy();

    m_datareadershm_map.erase(iter);
  }
}
