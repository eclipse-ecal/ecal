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
 * @brief  eCAL registration receiver
 *
 * Receives registration information from external eCAL processes and forwards them to
 * the internal publisher/subscriber, server/clients.
 *
**/

#include "ecal_globals.h"

#include "ecal_registration_receiver_shm.h"
#include "serialization/ecal_serialize_sample_registration.h"
#include <chrono>
#include <functional>
#include <memory>

namespace eCAL
{
  //////////////////////////////////////////////////////////////////
  // CMemfileRegistrationReceiver
  //////////////////////////////////////////////////////////////////

  CMemfileRegistrationReceiver::~CMemfileRegistrationReceiver()
  {
    Destroy();
  }

  void CMemfileRegistrationReceiver::Create(eCAL::CMemoryFileBroadcastReader* memfile_broadcast_reader_)
  {
    if (m_created) return;

    // start memfile broadcast receive thread
    m_memfile_broadcast_reader = memfile_broadcast_reader_;
    m_memfile_broadcast_reader_thread = std::make_shared<CCallbackThread>(std::bind(&CMemfileRegistrationReceiver::Receive, this));
    m_memfile_broadcast_reader_thread->start(std::chrono::milliseconds(Config::GetRegistrationRefreshMs()/2));

    m_created = true;
  }

  void CMemfileRegistrationReceiver::Destroy()
  {
    if (!m_created) return;

    // stop memfile broadcast receive thread
    m_memfile_broadcast_reader_thread->stop();
    m_memfile_broadcast_reader = nullptr;

    m_created = false;
  }

  void CMemfileRegistrationReceiver::Receive()
  {
    MemfileBroadcastMessageListT message_list;
    if (m_memfile_broadcast_reader->Read(message_list, 0))
    {
      eCAL::Registration::SampleList sample_list;
      for (const auto& message : message_list)
      {
        if (DeserializeFromBuffer(static_cast<const char*>(message.data), message.size, sample_list))
        {
          for (const auto& sample : sample_list.samples)
          {
            if (g_registration_receiver() != nullptr) g_registration_receiver()->ApplySample(sample);
          }
        }
      }
    }
  }
}
