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
 * Receives registration information from the sample applier and forwards them to
 * user defined callbacks.
**/

#pragma once

#include <cstddef>
#include <ecal/ecal.h>

#include "serialization/ecal_struct_sample_registration.h"

namespace eCAL
{
  namespace Registration
  {
    class CSampleApplierUser
    {
    public:
      bool AddRegistrationCallback(enum eCAL_Registration_Event event_, const RegistrationCallbackT& callback_);
      bool RemRegistrationCallback(enum eCAL_Registration_Event event_);

      void ApplySample(const eCAL::Registration::Sample& sample_);

    private:
      // in the future this may be stored in a map? or somehow differently
      RegistrationCallbackT  m_callback_pub = nullptr;
      RegistrationCallbackT  m_callback_sub = nullptr;
      RegistrationCallbackT  m_callback_service = nullptr;
      RegistrationCallbackT  m_callback_client = nullptr;
      RegistrationCallbackT  m_callback_process = nullptr;

      // protect by mutexes? very likeley need to!
    };
  }
}
