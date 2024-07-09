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
 * @brief  Definition of custom data types.
**/

#include <ecal/config/registration.h>
#include "ecal_def.h"

namespace eCAL
{
  namespace Registration
  {
    Configuration::Configuration()
    : network_enabled(NET_ENABLED)
    , shm_registration_enabled(SHM_REGISTRATION_ENABLED)
    , share_ttype(PUB_SHARE_TTYPE)
    , share_tdesc(PUB_SHARE_TDESC)
    , m_registration_timeout(CMN_REGISTRATION_TO)
    , m_registration_refresh(CMN_REGISTRATION_REFRESH)
    {}

    Configuration::Configuration(unsigned int reg_timeout_, unsigned int reg_refresh_)
    : network_enabled(NET_ENABLED)
    , shm_registration_enabled(SHM_REGISTRATION_ENABLED)
    , share_ttype(PUB_SHARE_TTYPE)
    , share_tdesc(PUB_SHARE_TDESC)
    {
      if (reg_refresh_ < reg_timeout_)
      {
        m_registration_timeout = reg_timeout_;
        m_registration_refresh = reg_refresh_;
      }
      else
      {
        throw std::invalid_argument("[RegistrationOptions] Refresh(" + std::to_string(reg_refresh_) + ") >= registration timeout (" + std::to_string(reg_timeout_) + ").");
      }
    }

    unsigned int Configuration::getTimeoutMS() const { return m_registration_timeout; }
    unsigned int Configuration::getRefreshMS() const { return m_registration_refresh; }
  }
}