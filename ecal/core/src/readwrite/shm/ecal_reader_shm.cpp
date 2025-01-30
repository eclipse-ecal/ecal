/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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
  ////////////////
  // LAYER
  ////////////////
  void CSHMReaderLayer::Initialize(const eCAL::eCALReader::SHM::SAttributes& attr_)
  {
    m_attributes = attr_;
  }

  void CSHMReaderLayer::SetConnectionParameter(SReaderLayerPar& par_)
  {
    for (const auto& memfile_name : par_.parameter.layer_par_shm.memory_file_list)
    {
      // start memory file receive thread if topic is subscribed in this process
      if (g_memfile_pool() != nullptr)
      { 
        const std::string process_id = std::to_string(m_attributes.process_id);
        const std::string memfile_event = memfile_name + "_" + process_id;

        Payload::TopicInfo topic_info;
        topic_info.topic_name = par_.topic_name;
        topic_info.host_name  = par_.host_name;
        topic_info.topic_id   = par_.topic_id;
        topic_info.process_id = par_.process_id;

        auto data_callback = [this, topic_info](const char* buf_, size_t len_, long long id_, long long clock_, long long time_, size_t hash_)->size_t
        {
          return OnNewShmFileContent(topic_info, buf_, len_, id_, clock_, time_, hash_);
        };
        g_memfile_pool()->ObserveFile(memfile_name, memfile_event, m_attributes.registration_timeout_ms, data_callback);
      }
    }
  }

  size_t CSHMReaderLayer::OnNewShmFileContent(const Payload::TopicInfo& topic_info_, const char* buf_, size_t len_, long long id_, long long clock_, long long time_, size_t hash_)
  {
    if (g_subgate() != nullptr)
    {
      if (g_subgate()->ApplySample(topic_info_, buf_, len_, id_, clock_, time_, hash_, tl_ecal_shm))
      {
        return len_;
      }
    }
    return 0;
  }
}
