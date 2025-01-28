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
 * @brief  memory file data writer
**/

#pragma once

#include "config/attributes/writer_shm_attributes.h"

#include "io/shm/ecal_memfile_sync.h"
#include "readwrite/ecal_writer_base.h"

#include <cstddef>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

namespace eCAL
{
  class CDataWriterSHM : public CDataWriterBase
  {
  public:
    CDataWriterSHM(const eCALWriter::SHM::SAttributes& attr_);

    SWriterInfo GetInfo() override;

    bool PrepareWrite(const SWriterAttr& attr_) override;

    bool Write(CPayloadWriter& payload_, const SWriterAttr& attr_) override;

    void ApplySubscription(const std::string& host_name_, int32_t process_id_, const EntityIdT& topic_id_, const std::string& conn_par_) override;
    void RemoveSubscription(const std::string& host_name_, int32_t process_id_, const EntityIdT& topic_id_) override;

    Registration::ConnectionPar GetConnectionParameter() override;

  protected:
    bool SetBufferCount(size_t buffer_count_);

    eCALWriter::SHM::SAttributes                  m_attributes;

    size_t                                        m_write_idx = 0;
    std::vector<std::shared_ptr<CSyncMemoryFile>> m_memory_file_vec;
    static const std::string                      m_memfile_base_name;

    using ProcessIDTopicIDSetT = std::map<int32_t, std::set<EntityIdT>>;
    std::mutex                                    m_process_id_topic_id_set_map_sync;
    ProcessIDTopicIDSetT                          m_process_id_topic_id_set_map;
  };
}
