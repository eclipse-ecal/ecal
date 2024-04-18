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
 * @brief  shared memory layer
**/

#include <ecal/ecal.h>
#include <functional>
#include <string>

#include "ecal_global_accessors.h"
#include "pubsub/ecal_subgate.h"
#include "io/shm/ecal_memfile_pool.h"
#include "ecal_reader_shm.h"

namespace eCAL
{
  ////////////////
  // LAYER
  ////////////////
  void CSHMReaderLayer::SetConnectionParameter(SReaderLayerPar& par_)
  {
    for (const auto& memfile_name : par_.parameter.layer_par_shm.memory_file_list)
    {
      // start memory file receive thread if topic is subscribed in this process
      if (g_memfile_pool() != nullptr)
      {
        const std::string process_id    = std::to_string(Process::GetProcessID());
        const std::string memfile_event = memfile_name + "_" + process_id;
        const MemFileDataCallbackT memfile_data_callback = std::bind(&CSHMReaderLayer::OnNewShmFileContent, this,
          std::placeholders::_1,
          std::placeholders::_2,
          std::placeholders::_3,
          std::placeholders::_4,
          std::placeholders::_5,
          std::placeholders::_6,
          std::placeholders::_7,
          std::placeholders::_8);
        g_memfile_pool()->ObserveFile(memfile_name, memfile_event, par_.topic_name, par_.topic_id, Config::GetRegistrationTimeoutMs(), memfile_data_callback);
      }
    }
  }

  size_t CSHMReaderLayer::OnNewShmFileContent(const std::string& topic_name_, const std::string& topic_id_, const char* buf_, size_t len_, long long id_, long long clock_, long long time_, size_t hash_)
  {
    if (g_subgate() != nullptr)
    {
      if (g_subgate()->ApplySample(topic_name_, topic_id_, buf_, len_, id_, clock_, time_, hash_, tl_ecal_shm))
      {
        return len_;
      }
    }
    return 0;
  }
}
