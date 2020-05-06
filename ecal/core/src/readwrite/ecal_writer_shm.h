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

#include "ecal_def.h"

#include "readwrite/ecal_writer_base.h"
#include "io/ecal_memfile.h"

#include <ecal/ecal_eventhandle.h>

#include <mutex>
#include <string>
#include <unordered_map>
#include <atomic>

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

    bool PrepareSend(size_t len_) override;
    size_t Send(const SWriterData& data_) override;

    bool AddLocConnection(const std::string& process_id_, const std::string& conn_par_) override;
    bool RemLocConnection(const std::string& process_id_) override;

    std::string GetConectionPar() override;

  protected:
    void SignalMemFileWritten();

    void BuildMemFileName();
    bool CreateMemFile(size_t size_);
    bool DestroyMemFile();

    std::string      m_memfile_name;
    CMemoryFile      m_memfile;

    struct SEventHandlePair
    {
      EventHandleT event_snd;
      EventHandleT event_ack;
    };
    typedef std::unordered_map<std::string, SEventHandlePair> EventHandleMapT;
    std::mutex       m_event_handle_map_sync;
    EventHandleMapT  m_event_handle_map;

    int              m_timeout_ack;
  };
}
