/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 * Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
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

#include <ecal/config.h>
#include <ecal/process.h>

#include "ecal_global_accessors.h"
#include "ecal_reader_shm.h"

#include "io/shm/ecal_memfile_pool.h"
#include "pubsub/ecal_subgate.h"

#include <functional>
#include <string>

namespace eCAL
{
  CSHMReaderLayer::CSHMReaderLayer(const eCAL::eCALReader::SHM::SAttributes& attr_) :
    CTransportLayerInstance(LayerType::SHM),
    m_attributes(attr_)
  {
  }

  bool CSHMReaderLayer::AcceptsConnection(const PublisherConnectionParameters& publisher, const SubscriberConnectionParameters& subscriber) const
  {
    // We need to check if publisher and subscriber both have SHM layer active
    // We also need to check if they are the same SHM memory domain, otherwise they will not be able to communicate
    
    if (!LayerEnabledForPublisherAndSubscriber(m_layer_type, publisher, subscriber))
    {
      return false;
    }
    if (publisher.GetSHMTransportDomain() != subscriber.GetSHMTransportDomain())
    {
      return false;
    }
    return true;
  }

  CTransportLayerInstance::ConnectionToken CSHMReaderLayer::AddConnection(const PublisherConnectionParameters& publisher, const ReceiveCallbackT& on_data, const ConnectionChangeCallback& on_connection_changed)
  {
    for (const auto& memfile_name : publisher.GetLayerParameter(m_layer_type).par_layer.layer_par_shm.memory_file_list)
    {
      // start memory file receive thread if topic is subscribed in this process
      // TODO we should be able to eliminate the g_memfile_pool. Instead the CSHMReaderLayer should own a memfile pool instance.
      auto memfile_pool = g_memfile_pool();
      if (memfile_pool)
      {
        const std::string process_id = std::to_string(m_attributes.process_id);
        const std::string memfile_event = memfile_name + "_" + process_id;

        STopicId publisher_topic_id = publisher.GetTopicId();
        SDataTypeInformation publisher_datatype_info = publisher.GetDataTypeInformation();

        auto data_callback = [on_data, publisher_topic_id, publisher_datatype_info](const char* buf_, size_t len_, long long id_, long long clock_, long long time_, size_t hash_)->size_t
        {
          SReceiveCallbackData data;
          data.buffer = buf_;
          data.buffer_size = len_;
          data.send_clock = clock_;
          data.send_timestamp = time_;
          on_data(publisher_topic_id, publisher_datatype_info, data);
          return len_;
        };
        memfile_pool->ObserveFile(memfile_name, memfile_event, m_attributes.registration_timeout_ms, data_callback);
        // TODO: when is a connection actually established? We also need to trigger the connection callback
      }
    }

    return ConnectionToken();
  }

}
