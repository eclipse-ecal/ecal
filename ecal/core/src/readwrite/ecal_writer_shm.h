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
    CDataWriterSHM();
    ~CDataWriterSHM();

    void GetInfo(SWriterInfo info_) override;

    bool Create(const std::string& host_name_, const std::string& topic_name_, const std::string & topic_id_) override;
    bool Destroy() override;

    bool SetQOS(const QOS::SWriterQOS& qos_) override;

    bool PrepareWrite(const SWriterData& data_) override;
    bool Write(const SWriterData& data_) override;

    bool AddLocConnection(const std::string& process_id_, const std::string& conn_par_) override;
    bool RemLocConnection(const std::string& process_id_) override;

    std::string GetConnectionParameter() override;

  protected:
    size_t                                        m_write_idx    = 0;
    size_t                                        m_buffer_count = 1;
    std::vector<std::shared_ptr<CSyncMemoryFile>> m_memory_file_vec;
  };
}
