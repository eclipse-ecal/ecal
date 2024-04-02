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

#pragma once

#include "shm/ecal_memfile_broadcast.h"
#include "shm/ecal_memfile_broadcast_reader.h"

#include "util/ecal_thread.h"
#include <memory>

namespace eCAL
{
  class CMemfileRegistrationReceiver
  {
  public:
    CMemfileRegistrationReceiver() = default;
    ~CMemfileRegistrationReceiver();

    // default copy constructor
    CMemfileRegistrationReceiver(const CMemfileRegistrationReceiver& other) = delete;
    // default copy assignment operator
    CMemfileRegistrationReceiver& operator=(const CMemfileRegistrationReceiver& other) = delete;
    // default move constructor
    CMemfileRegistrationReceiver(CMemfileRegistrationReceiver&& other) noexcept = delete;
    // default move assignment operator
    CMemfileRegistrationReceiver& operator=(CMemfileRegistrationReceiver&& other) noexcept = delete;

    void Create(CMemoryFileBroadcastReader* memfile_broadcast_reader_);
    void Destroy();

  private:
    void Receive();

    CMemoryFileBroadcastReader*       m_memfile_broadcast_reader = nullptr;
    std::shared_ptr<CCallbackThread>  m_memfile_broadcast_reader_thread;

    bool m_created = false;
  };
}
