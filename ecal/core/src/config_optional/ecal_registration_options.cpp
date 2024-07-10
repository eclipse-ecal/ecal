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

#include <ecal/config_optional/registration.h>

namespace eCAL
{
  namespace Registration
  {
    Configuration::Configuration()
    {}

    Configuration::Configuration(unsigned int reg_timeout_, unsigned int reg_refresh_)
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

    std::optional<unsigned int> Configuration::getTimeoutMS() const { return m_registration_timeout; }
    std::optional<unsigned int> Configuration::getRefreshMS() const { return m_registration_refresh; }
  }
}