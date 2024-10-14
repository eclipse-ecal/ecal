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
 * @brief  eCAL UDP registration receiver
 *
 * Handles UDP samples coming from other processes
 *
**/

#include <memory>
#include <registration/ecal_registration_types.h>
#include "registration/udp/config/attributes/registration_receiver_udp_attributes.h"

namespace eCAL
{
  namespace UDP
  {
    class CSampleReceiver;
  }

  class CRegistrationReceiverUDP
  {
  public:
    CRegistrationReceiverUDP(RegistrationApplySampleCallbackT apply_sample_callback, const Registration::UDP::SReceiverAttributes& attr_);
    ~CRegistrationReceiverUDP();

    // Special member functionss
    CRegistrationReceiverUDP(const CRegistrationReceiverUDP& other) = delete;
    CRegistrationReceiverUDP& operator=(const CRegistrationReceiverUDP& other) = delete;
    CRegistrationReceiverUDP(CRegistrationReceiverUDP&& other) noexcept = delete;
    CRegistrationReceiverUDP& operator=(CRegistrationReceiverUDP&& other) noexcept = delete;

  private:
    std::unique_ptr<UDP::CSampleReceiver> m_registration_receiver;
  };
}