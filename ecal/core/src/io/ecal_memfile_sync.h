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
 * @brief  synchronized memory file interface
**/

#pragma once

#include <ecal/ecal_eventhandle.h>
#include <ecal/ecal_payload_writer.h>

#include "readwrite/ecal_writer_data.h"
#include "ecal_memfile.h"

#include <mutex>
#include <string>
#include <unordered_map>

namespace eCAL
{
  struct SSyncMemoryFileAttr
  {
    size_t  min_size;           //!< memory file minimum size [Bytes]
    size_t  reserve;            //!< dynamic file size reserve before recreating memory file if payload size changes [%]
    int64_t timeout_open_ms;    //!< timeout to open a memory file using mutex lock [ms]
    int64_t timeout_ack_ms;     //!< timeout for memory read acknowledge signal from data reader [ms]
  };

  class CSyncMemoryFile
  {
  public:
    CSyncMemoryFile(const std::string& base_name_, size_t size_, SSyncMemoryFileAttr attr_);
    ~CSyncMemoryFile();

    bool Connect(const std::string& process_id_);
    bool Disconnect(const std::string& process_id_);

    bool CheckSize(size_t size_);
    bool Write(CPayloadWriter& payload_, const SWriterAttr& data_, bool force_full_write_ = false);

    std::string GetName() const;
    size_t GetSize() const;
    bool IsCreated() const { return m_created; };

  protected:
    bool Create(const std::string& base_name_, size_t size_);
    bool Destroy();
    bool Recreate(size_t size_);

    void SyncContent();
    void DisconnectAll();

    std::string         m_base_name;
    std::string         m_memfile_name;
    CMemoryFile         m_memfile;
    SSyncMemoryFileAttr m_attr;
    bool                m_created;

    struct SEventHandlePair
    {
      EventHandleT event_snd;
      EventHandleT event_ack;
      bool         event_ack_is_invalid = false;    //!< The ack event has timeouted. Thus, we don't wait for it anymore, until the subscriber notifies us via registration layer that it is still alive.
    };
    using EventHandleMapT = std::unordered_map<std::string, SEventHandlePair>;
    std::mutex       m_event_handle_map_sync;
    EventHandleMapT  m_event_handle_map;
  };
}
