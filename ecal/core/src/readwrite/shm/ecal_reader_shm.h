/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 * Copyright 2026 AUMOVIO and subsidiaries. All rights reserved.
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

#pragma once

#include "ecal_def.h"
#include "readwrite/ecal_reader_layer.h"
#include "serialization/ecal_struct_sample_payload.h"
#include "config/attributes/reader_shm_attributes.h"

#include <cstddef>
#include <memory>
#include <string>

namespace eCAL
{
  class CSubGate;
  class CMemfileThreadPool;

  ////////////////
  // LAYER
  ////////////////
  class CSHMReaderLayer : public CReaderLayer<CSHMReaderLayer, eCAL::eCALReader::SHM::SAttributes>
  {
  public:
    CSHMReaderLayer(std::shared_ptr<eCAL::CSubGate> subgate_, std::shared_ptr<eCAL::CMemFileThreadPool> memfile_thread_pool_);

    void Initialize(const eCAL::eCALReader::SHM::SAttributes& attr_) override;
    void AddSubscription(const std::string& /*host_name_*/, const std::string& /*topic_name_*/, const EntityIdT& /*topic_id_*/) override {}
    void RemSubscription(const std::string& /*host_name_*/, const std::string& /*topic_name_*/, const EntityIdT& /*topic_id_*/) override {}

    void SetConnectionParameter(SReaderLayerPar& par_) override;

  private:
    size_t OnNewShmFileContent(const Payload::TopicInfo& topic_info_, const char* buf_, size_t len_, long long id_, long long clock_, long long time_, size_t hash_);

    eCAL::eCALReader::SHM::SAttributes        m_attributes;
    std::shared_ptr<eCAL::CSubGate>           m_subgate;
    std::shared_ptr<eCAL::CMemFileThreadPool> m_memfile_thread_pool;
  };
}
