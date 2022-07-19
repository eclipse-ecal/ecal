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
 * @brief  synchronized memory file
**/

#include <ecal/ecal.h>
#include <ecal/ecal_config.h>
#include <ecal/ecal_log.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include "ecal/pb/layer.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "ecal_def.h"
#include "ecal_config_reader_hlp.h"

#include "io/ecal_memfile_header.h"
#include "io/ecal_memfile_sync.h"

#include <algorithm>
#include <sstream>
#include <chrono>

namespace eCAL
{
  CSyncMemoryFile::CSyncMemoryFile() :
    m_timeout_ack(PUB_MEMFILE_ACK_TO)
  {
  }

  CSyncMemoryFile::~CSyncMemoryFile()
  {
    Destroy();
  }

  bool CSyncMemoryFile::Create(const std::string& base_name_, size_t size_)
  {
    if (m_created) return false;

    m_base_name   = base_name_;
    m_timeout_ack = Config::GetMemfileAckTimeoutMs();

    // build unique memory file name
    BuildMemFileName();

    // create new memory file object
    // with additional space for SMemFileHeader
    size_t memfile_size = sizeof(SMemFileHeader) + size_;
    // check for minimal size
    size_t minsize = Config::GetMemfileMinsizeBytes();
    if (memfile_size < minsize) memfile_size = minsize;

    // create the memory file
    if (!m_memfile.Create(m_memfile_name.c_str(), true, memfile_size))
    {
      // log it
      Logging::Log(log_level_error, std::string(m_base_name + "::CSyncMemoryFile::Create - FAILED : ") + m_memfile_name);

      return false;
    }

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug2, std::string(m_base_name + "::CSyncMemoryFile::Create - SUCCESS : ") + m_memfile_name);
#endif

    // initialize memory file with empty header
    struct SMemFileHeader memfile_hdr;
    m_memfile.GetWriteAccess(PUB_MEMFILE_OPEN_TO);
    m_memfile.Write(&memfile_hdr, memfile_hdr.hdr_size, 0);
    m_memfile.ReleaseWriteAccess();

    // it's created
    m_created = true;

    return true;
  }

  bool CSyncMemoryFile::Destroy()
  {
    if (!m_created) return false;

    // state destruction in progress
    m_created = false;

    // reset memory file name
    m_memfile_name.clear();

    // disconnect all processes
    DisconnectAllProcesses();

    // destroy the file
    if (!m_memfile.Destroy(true))
    {
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug2, std::string(m_base_name + "::CSyncMemoryFile::Destroy - FAILED : ") + m_memfile.Name());
#endif
      return false;
    }

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug2, std::string(m_base_name + "::CSyncMemoryFile::Destroy - SUCCESS : ") + m_memfile.Name());
#endif
    return true;
  }

  bool CSyncMemoryFile::ConnectProcess(const std::string& process_id_)
  {
    if (!m_created) return false;

    // a local subscriber is registering with it's process id
    // so we have to check the sync events for the
    // memory content read / write access
    // we have ONE memory file per publisher and ONE
    // memory file read thread per subscriber

    std::lock_guard<std::mutex> lock(m_event_handle_map_sync);
    EventHandleMapT::iterator iter = m_event_handle_map.find(process_id_);
    std::string event_ack_name = m_memfile_name + "_" + process_id_ + "_ack";

    // add a new process id and create the sync and acknowledge event
    if (iter == m_event_handle_map.end())
    {
      SEventHandlePair event_pair;
      std::string event_snd_name = m_memfile_name + "_" + process_id_;
      gOpenEvent(&event_pair.event_snd, event_snd_name);
      if (m_timeout_ack != 0)
      {
        gOpenEvent(&event_pair.event_ack, event_ack_name);
      }
      m_event_handle_map.insert(std::pair<std::string, SEventHandlePair>(process_id_, event_pair));
      return true;
    }
    else
    {
      if (m_timeout_ack != 0)
      {
        // okay we have registered process events for that process id
        // we have to check the acknowledge event because it's possible that this
        // event was deactivated by a sync timeout in SignalMemFileWritten
        if (!gEventIsValid(iter->second.event_ack))
        {
          gOpenEvent(&iter->second.event_ack, event_ack_name);
        }
      }
      return true;
    }
  }

  bool CSyncMemoryFile::DisconnectProcess(const std::string& process_id_)
  {
    if (!m_created) return false;

    // a local subscriber connection timed out
    // we close the associated sync events and
    // remove them from the event handle map

    std::lock_guard<std::mutex> lock(m_event_handle_map_sync);
    EventHandleMapT::const_iterator iter = m_event_handle_map.find(process_id_);
    if (iter != m_event_handle_map.end())
    {
      SEventHandlePair event_pair = iter->second;
      gCloseEvent(event_pair.event_snd);
      if (m_timeout_ack != 0)
      {
        gCloseEvent(event_pair.event_ack);
      }
      m_event_handle_map.erase(iter);
      return true;
    }

    return false;
  }

  bool CSyncMemoryFile::CheckSize(size_t size_)
  {
    if (!m_created) return false;

    // we recreate a memory file if the file size is to small
    bool file_to_small = m_memfile.MaxDataSize() < (sizeof(SMemFileHeader) + size_);
    if (file_to_small)
    {
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug4, m_base_name + "::CSyncMemoryFile::CheckSize - RECREATE");
#endif
      // estimate size of memory file
      size_t memfile_reserve = Config::GetMemfileOverprovisioningPercentage();
      size_t memfile_size    = sizeof(SMemFileHeader) + size_ + static_cast<size_t>((static_cast<float>(memfile_reserve) / 100.0f) * static_cast<float>(size_));

      // recreate the file
      if (!RecreateFile(memfile_size)) return false;

      // return true to trigger registration and immediately inform listening subscribers
      return true;
    }

    return false;
  }

  bool CSyncMemoryFile::Write(const CDataWriterBase::SWriterData& data_)
  {
    if (!m_created) return false;

    // write header and payload into the memory file

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug4, m_base_name + "::CSyncMemoryFile::Write");
#endif

    // created ?
    if (!m_memfile.IsCreated())
    {
      // log it
      Logging::Log(log_level_error, m_base_name + "::CSyncMemoryFile::Write::IsCreated - FAILED");
      return false;
    }

    // create user file header
    struct SMemFileHeader memfile_hdr;
    // set data size
    memfile_hdr.data_size         = static_cast<unsigned long>(data_.len);
    // set header id
    memfile_hdr.id                = static_cast<unsigned long>(data_.id);
    // set header clock
    memfile_hdr.clock             = static_cast<unsigned long>(data_.clock);
    // set header time
    memfile_hdr.time              = static_cast<long long>(data_.time);
    // set header hash
    memfile_hdr.hash              = static_cast<size_t>(data_.hash);
    // set zero copy
    memfile_hdr.options.zero_copy = static_cast<unsigned char>(data_.zero_copy);

    // open the memory file
    bool opened = m_memfile.GetWriteAccess(PUB_MEMFILE_OPEN_TO);

    // maybe it's locked by a zombie or a crashed process
    // so we try to recreate a new one
    if (!opened)
    {
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug2, m_base_name + "::CSyncMemoryFile::Write::Open - FAILED");
#endif

      // try to recreate the memory file
      if (!RecreateFile(m_memfile.MaxDataSize())) return false;

      // then reopen
      opened = m_memfile.GetWriteAccess(PUB_MEMFILE_OPEN_TO);
      // still no chance ? hell .... we give up
      if (!opened) return false;
    }

    // now write content
    bool written(true);
    size_t wbytes(0);

    // write the user file header
    written &= m_memfile.Write(&memfile_hdr, memfile_hdr.hdr_size, wbytes) > 0;
    wbytes += memfile_hdr.hdr_size;
    // write the buffer
    if (data_.len > 0)
    {
      written &= m_memfile.Write(data_.buf, data_.len, wbytes) > 0;
    }
    // close memory file
    m_memfile.ReleaseWriteAccess();

    // and fire the publish event for local subscriber
    if (written) SignalWritten();

#ifndef NDEBUG
    // log it
    if (written)
    {
      Logging::Log(log_level_debug4, m_base_name + "::CSyncMemoryFile::Write::Written (" + std::to_string(data_.len) + " Bytes)");
    }
    else
    {
      Logging::Log(log_level_error, m_base_name + "::CSyncMemoryFile::Write::Written - FAILED");
    }
#endif

    // return success
    return written;
  }

  std::string CSyncMemoryFile::GetName()
  {
    return m_memfile_name;
  }

  bool CSyncMemoryFile::RecreateFile(size_t size_)
  {
    // collect id's of the currently connected processes
    std::vector<std::string> process_id_list;
    {
      std::lock_guard<std::mutex> lock(m_event_handle_map_sync);
      for (auto iter : m_event_handle_map)
      {
        process_id_list.push_back(iter.first);
      }
    }

    // destroy existing memory file object
    Destroy();

    // create a new one
    if (!Create(m_base_name, size_)) return false;

    // reconnect processes
    for (auto process_id : process_id_list)
    {
      ConnectProcess(process_id);
    }

    return true;
  }

  void CSyncMemoryFile::SignalWritten()
  {
    if (!m_created) return;

    // fire the publisher events
    // connected subscribers will read the content from the memory file

    std::lock_guard<std::mutex> lock(m_event_handle_map_sync);

    // "eat" old acknowledge events :)
    if (m_timeout_ack != 0)
    {
      for (auto iter : m_event_handle_map)
      {
        while (gWaitForEvent(iter.second.event_ack, 0)) {}
      }
    }

    // send new sync
    for (auto iter = m_event_handle_map.begin(); iter != m_event_handle_map.end(); ++iter)
    {
      // send sync event
      gSetEvent(iter->second.event_snd);

      // sync on acknowledge event
      if (m_timeout_ack != 0)
      {
        if (!gWaitForEvent(iter->second.event_ack, m_timeout_ack))
        {
          // we close the event immediately to not waste time in the next
          // write call, the event will be reopened later
          // in ApplyLocSubscription if the connection still exists
          gCloseEvent(iter->second.event_ack);
          // invalidate it
          gInvalidateEvent(&iter->second.event_ack);
#ifndef NDEBUG
          // log it
          Logging::Log(log_level_debug2, m_base_name + "::CSyncMemoryFile::SignalWritten - ACK event timeout");
#endif
        }
      }

#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug4, m_base_name + "::CSyncMemoryFile::SignalWritten");
#endif
    }
  }

  void CSyncMemoryFile::BuildMemFileName()
  {
    std::stringstream out;
    out << m_base_name << "_" << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    m_memfile_name = out.str();

    // replace all '\\' and '/' to '_'
    std::replace(m_memfile_name.begin(), m_memfile_name.end(), '\\', '_');
    std::replace(m_memfile_name.begin(), m_memfile_name.end(), '/', '_');

    // append "_shm" for debugging purposes
    m_memfile_name += "_shm";
  }

  void CSyncMemoryFile::DisconnectAllProcesses()
  {
    if (m_timeout_ack != 0)
    {
      // fire acknowledge events, to unlock blocking send function
      std::lock_guard<std::mutex> lock(m_event_handle_map_sync);
      for (auto iter : m_event_handle_map)
      {
        gSetEvent(iter.second.event_ack);
      }
    }

    // close all events, invalidate them and clear the map
    {
      std::lock_guard<std::mutex> lock(m_event_handle_map_sync);
      for (auto iter = m_event_handle_map.begin(); iter != m_event_handle_map.end(); ++iter)
      {
        gCloseEvent(iter->second.event_snd);
        gInvalidateEvent(&iter->second.event_snd);
        if (m_timeout_ack != 0)
        {
          gCloseEvent(iter->second.event_ack);
          gInvalidateEvent(&iter->second.event_ack);
        }
      }
      m_event_handle_map.clear();
    }
  }
}
