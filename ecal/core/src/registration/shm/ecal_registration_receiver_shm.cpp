/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
 * @brief  eCAL registration receiver
 *
 * Receives registration information from external eCAL processes and forwards them to
 * the internal publisher/subscriber, server/clients.
 *
**/

#include "ecal_globals.h"

#include "registration/shm/ecal_registration_receiver_shm.h"
#include "serialization/ecal_serialize_sample_registration.h"

#include "registration/shm/ecal_memfile_broadcast.h"
#include "registration/shm/ecal_memfile_broadcast_reader.h"
#include "util/ecal_thread.h"

namespace eCAL
{
  //////////////////////////////////////////////////////////////////
  // CMemfileRegistrationReceiver
  //////////////////////////////////////////////////////////////////

  CRegistrationReceiverSHM::CRegistrationReceiverSHM(RegistrationApplySampleCallbackT apply_sample_callback, const Registration::SHM::SAttributes& attr_)
   : m_apply_sample_callback(apply_sample_callback)
  {
    m_memfile_broadcast = std::make_unique<CMemoryFileBroadcast>();
    m_memfile_broadcast->Create(attr_);
    m_memfile_broadcast->FlushLocalEventQueue();

    m_memfile_broadcast_reader = std::make_unique<CMemoryFileBroadcastReader>();
    // This is a bit unclean to take the raw adress of the reader here.
    m_memfile_broadcast_reader->Bind(m_memfile_broadcast.get());

    m_memfile_broadcast_reader_thread = std::make_unique<CCallbackThread>(std::bind(&CRegistrationReceiverSHM::Receive, this));
    m_memfile_broadcast_reader_thread->start(std::chrono::milliseconds(Config::GetRegistrationRefreshMs() / 2));
  }

  CRegistrationReceiverSHM::~CRegistrationReceiverSHM()
  {
    m_memfile_broadcast_reader_thread->stop();
    m_memfile_broadcast_reader_thread = nullptr;

    // stop memfile registration receive thread and unbind reader
    m_memfile_broadcast_reader->Unbind();
    m_memfile_broadcast_reader = nullptr;

    m_memfile_broadcast->Destroy();
    m_memfile_broadcast = nullptr;
  }

  void CRegistrationReceiverSHM::Receive()
  {
    // At the moment this function is called synchronously by a dedicated thread.
    // If this changes, we need to protect the sample list member variable
    MemfileBroadcastMessageListT message_list;
    if (m_memfile_broadcast_reader->Read(message_list, 0))
    {
      m_sample_list.clear();
      for (const auto& message : message_list)
      {
        if (DeserializeFromBuffer(static_cast<const char*>(message.data), message.size, m_sample_list))
        {
          for (const auto& sample : m_sample_list)
          {
            m_apply_sample_callback(sample);
          }
        }
      }
    }
  }
}
