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
 * @brief  memory file data writer
**/

#pragma once

#include "readwrite/ecal_writer_base.h"
#include "io/ecal_memfile_sync.h"

#include <memory>
#include <string>

namespace eCAL
{
  class CDataWriterSHM : public CDataWriterBase
  {
  public:
    CDataWriterSHM() = default;
    ~CDataWriterSHM() override;

    SWriterInfo GetInfo() override;

    bool Create(const std::string& host_name_, const std::string& topic_name_, const std::string & topic_id_) override;
    // this virtual function is called during construction/destruction,
    // so, mark it as final to ensure that no derived classes override it.
    bool Destroy() final;

    bool SetQOS(const QOS::SWriterQOS& qos_) override;
    bool SetBufferCount(size_t buffer_count_);

    bool PrepareWrite(const SWriterAttr& attr_) override;

    bool Write(CPayloadWriter& payload_, const SWriterAttr& attr_) override;

    bool AddLocConnection(const std::string& process_id_, const std::string& conn_par_) override;
    bool RemLocConnection(const std::string& process_id_) override;

    std::string GetConnectionParameter() override;

  protected:      
    size_t                                        m_write_idx    = 0;
    size_t                                        m_buffer_count = 1;
    SSyncMemoryFileAttr                           m_memory_file_attr = {};
    std::vector<std::shared_ptr<CSyncMemoryFile>> m_memory_file_vec;
    static const std::string                      m_memfile_base_name;
  };
}
