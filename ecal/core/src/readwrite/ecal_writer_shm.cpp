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

#include <ecal/ecal.h>
#include <ecal/ecal_log.h>
#include <ecal/ecal_process.h>
#include <ecal/ecal_event.h>

#include "ecal_def.h"
#include "ecal_config_hlp.h"
#include "ecal_writer.h"
#include "ecal_writer_shm.h"

#include "io/ecal_message.h"

#include <algorithm>
#include <sstream>
#include <chrono>

namespace eCAL
{
  CDataWriterSHM::CDataWriterSHM() : 
    m_timeout_ack(PUB_MEMFILE_ACK_TO)
  {
  }
  
  CDataWriterSHM::~CDataWriterSHM()
  {
    Destroy();
  }

  void CDataWriterSHM::GetInfo(SWriterInfo info_)
  {
    info_.name                 = "shm";
    info_.description          = "Local shared memory data writer";

    info_.has_mode_local       = true;
    info_.has_mode_cloud       = false;

    info_.has_qos_history_kind = false;
    info_.has_qos_reliability  = true;

    info_.send_size_max        = -1;
  }
  
  bool CDataWriterSHM::Create(const std::string& /*host_name_*/, const std::string& topic_name_, const std::string & /*topic_id_*/)
  {
    if (m_created) return false;

    m_topic_name = topic_name_;

    m_timeout_ack = eCALPAR(PUB, MEMFILE_ACK_TO);

    CreateMemFile(static_cast<size_t>(eCALPAR(PUB, MEMFILE_MINSIZE)));

    m_created = true;
    return true;
  }

  bool CDataWriterSHM::Destroy()
  {
    if (!m_created) return false;

    DestroyMemFile();

    m_created = false;
    return true;
  }

  bool CDataWriterSHM::SetQOS(const QOS::SWriterQOS& qos_)
  {
    m_qos = qos_;
    return true;
  }

  bool CDataWriterSHM::PrepareSend(size_t len_)
  {
    if (!m_created) return false;
    if (len_ == 0)  return false;

    // we recreate a memory file if the file size is to small
    bool file_to_small = m_memfile.FileSize() < (sizeof(SEcalMessage) + len_);
    if (!m_memfile.IsCreated() || file_to_small)
    {
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug4, m_topic_name + "::CDataWriter::PrepareSend::RecreateFile");
#endif
      // estimate size of memory file
      size_t memfile_reserve = static_cast<size_t>(eCALPAR(PUB, MEMFILE_RESERVE));
      size_t memfile_size = sizeof(SEcalMessage) + len_ + static_cast<size_t>((static_cast<float>(memfile_reserve) / 100.0f) * static_cast<float>(len_));
      // destroy existing memory file object
      DestroyMemFile();
      // and create a new one
      CreateMemFile(memfile_size);
      // return true to trigger registration and immediately inform listening subscribers
      return true;
    }

    return false;
  }

  /////////////////////////////////////////////////////////////////
  // write the content into the memory file
  /////////////////////////////////////////////////////////////////
  size_t CDataWriterSHM::Send(const SWriterData& data_)
  {
    if (!m_created)      return(0);
    if (!data_.buf)      return(0);
    if (data_.len == 0)  return(0);

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug4, m_topic_name + "::CDataWriter::Send::Write2MemFile");
#endif

    // created ?
    if (!m_memfile.IsCreated())
    {
      // log it
      Logging::Log(log_level_error, m_topic_name + "::CDataWriter::Send::Write2MemFile::IsCreated - FAILED");

      return(0);
    }

    // create message header
    struct SEcalMessage ecal_message;

    // set data size
    ecal_message.data_size = static_cast<unsigned long>(data_.len);
    // set header id
    ecal_message.id        = static_cast<unsigned long>(data_.id);
    // set header clock
    ecal_message.clock     = static_cast<unsigned long>(data_.clock);
    // set header time
    ecal_message.time      = static_cast<long long>(data_.time);
    // set header hash
    ecal_message.hash      = static_cast<size_t>(data_.hash);

    // open the memory file
    bool opened = m_memfile.Open(PUB_MEMFILE_OPEN_TO);

    // maybe it's locked by a zombie or a crashed process
    // so we try to recreate a new one
    if (!opened)
    {
      // log it
      Logging::Log(log_level_error, m_topic_name + "::CDataWriter::Send::Write2MemFile::OpenMemFile - FAILED");

      // store size of the memory file
      size_t memfile_size = m_memfile.FileSize();
      // destroy and
      DestroyMemFile();
      // recreate it with the same size
      if (!CreateMemFile(memfile_size)) return(0);
      // then reopen
      opened = m_memfile.Open(PUB_MEMFILE_OPEN_TO);
      // still no chance ? hell .... we give up
      if (!opened) return(0);
    }

    // now write content
    bool written(true);
    size_t wbytes(0);

    // write the header
    written &= m_memfile.Write(&ecal_message, ecal_message.hdr_size, wbytes) > 0;
    wbytes += ecal_message.hdr_size;
    // write the buffer
    written &= m_memfile.Write(data_.buf, data_.len, wbytes) > 0;
    // close memory file
    m_memfile.Close();

    // and fire the publish event for local subscriber
    if (written) SignalMemFileWritten();

#ifndef NDEBUG
    // log it
    if (written)
    {
      Logging::Log(log_level_debug4, m_topic_name + "::CDataWriter::Send::Write2MemFile::Written (" + std::to_string(data_.len) + " Bytes)");
    }
    else
    {
      Logging::Log(log_level_error, m_topic_name + "::CDataWriter::Send::Write2MemFile::Written - FAILED");
    }
#endif

    // if we failed return 0
    if (!written) return(0);

    // return success
    return(data_.len);
  }

  bool CDataWriterSHM::AddLocConnection(const std::string& process_id_, const std::string& /*conn_par_*/)
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

  bool CDataWriterSHM::RemLocConnection(const std::string& process_id_)
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

  std::string CDataWriterSHM::GetConectionPar()
  {
    // this is a workaround ! We need to pack all in a protocol buffer to more be flexible
    return m_memfile_name;
  }

  /////////////////////////////////////////////////////////////////
  // fire the publisher events
  // connected subscribers will read the content from the memory file
  /////////////////////////////////////////////////////////////////
  void CDataWriterSHM::SignalMemFileWritten()
  {
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
          Logging::Log(log_level_debug2, m_topic_name + "::CDataWriter::SignalMemFileWritten - ACK event timeout");
#endif
        }
      }

#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug4, m_topic_name + "::CDataWriter::SignalMemFileWritten");
#endif
    }
  }

  void CDataWriterSHM::BuildMemFileName()
  {
    std::stringstream out;
    out << m_topic_name << "_" << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    m_memfile_name = out.str();

    // replace all '\\' to '_'
    std::replace(m_memfile_name.begin(), m_memfile_name.end(), '\\', '_');

    // replace all '/' to '_'
    std::replace(m_memfile_name.begin(), m_memfile_name.end(), '/', '_');

    // append "_mem" for debugging puposes
    m_memfile_name += "_shm";
  }

  bool CDataWriterSHM::CreateMemFile(size_t size_)
  {
    // build new memory file name
    BuildMemFileName();

    // create new memory file object
    size_t minsize = static_cast<size_t>(eCALPAR(PUB, MEMFILE_MINSIZE));
    if (size_ < minsize) size_ = minsize;
    if (!m_memfile.Create(m_memfile_name.c_str(), true, size_))
    {
      // log it
      Logging::Log(log_level_error, std::string(m_topic_name + "::CDataWriter::CreateMemFile - FAILED : ") + m_memfile_name);

      return(false);
    }

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug2, std::string(m_topic_name + "::CDataWriter::CreateMemFile - SUCCESS : ") + m_memfile_name);
#endif

    // initialize memory file with empty header
    struct SEcalMessage ecal_message;
    m_memfile.Open(PUB_MEMFILE_OPEN_TO);
    m_memfile.Write(&ecal_message, ecal_message.hdr_size, 0);
    m_memfile.Close();

    // collect all connected process id's
    std::list<std::string> process_id_list;
    {
      std::lock_guard<std::mutex> lock(m_event_handle_map_sync);
      for (auto iter : m_event_handle_map)
      {
        process_id_list.push_back(iter.first);
      }
    }

    // and recreate immediately the new events
    for (auto process_id : process_id_list)
    {
      RemLocConnection(process_id);
      AddLocConnection(process_id, "");
    }

    return(true);
  }

  bool CDataWriterSHM::DestroyMemFile()
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

    // close all events and invalidate them
    // do not clear the map, because we will use
    // the map keys (process id's) to recreate the
    // events in a subsequent CreateMemFile call
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

    // destroy the file
    if (!m_memfile.Destroy(true))
    {
#ifndef NDEBUG
      // log it
      Logging::Log(log_level_debug2, std::string(m_topic_name + "::CDataWriter::DestroyMemFile - FAILED : ") + m_memfile.Name());
#endif
      // reset memory file name
      m_memfile_name.clear();

      return(false);
    }

#ifndef NDEBUG
    // log it
    Logging::Log(log_level_debug2, std::string(m_topic_name + "::CDataWriter::DestroyMemFile - SUCCESS : ") + m_memfile.Name());
#endif

    return(true);
  }
}
