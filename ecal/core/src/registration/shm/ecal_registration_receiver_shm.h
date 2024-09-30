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

#pragma once

#include <memory>
#include <registration/ecal_registration_types.h>
#include "config/attributes/registration_shm_attributes.h"

namespace eCAL
{
  class CCallbackThread;
  class CMemoryFileBroadcast;
  class CMemoryFileBroadcastReader;

  class CRegistrationReceiverSHM
  {
  public:
    CRegistrationReceiverSHM(RegistrationApplySampleCallbackT apply_sample_callback, const Registration::SHM::SAttributes& attr_);
    ~CRegistrationReceiverSHM();

    // default copy constructor
    CRegistrationReceiverSHM(const CRegistrationReceiverSHM& other) = delete;
    // default copy assignment operator
    CRegistrationReceiverSHM& operator=(const CRegistrationReceiverSHM& other) = delete;
    // default move constructor
    CRegistrationReceiverSHM(CRegistrationReceiverSHM&& other) noexcept = delete;
    // default move assignment operator
    CRegistrationReceiverSHM& operator=(CRegistrationReceiverSHM&& other) noexcept = delete;

  private:
    void Receive();

    std::unique_ptr<CMemoryFileBroadcast>       m_memfile_broadcast;
    std::unique_ptr<CMemoryFileBroadcastReader> m_memfile_broadcast_reader;
    std::unique_ptr<CCallbackThread>            m_memfile_broadcast_reader_thread;

    eCAL::Registration::SampleList              m_sample_list;

    RegistrationApplySampleCallbackT m_apply_sample_callback;
  };
}
