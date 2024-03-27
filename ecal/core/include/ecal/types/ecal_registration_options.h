/* =========================== LICENSE =================================
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
 * =========================== LICENSE =================================
 */

/**
 * @file   ecal_registration_options.h
 * @brief  eCAL options for configuration of the registration layer
**/

#pragma once

#include <chrono>
#include <iostream>

namespace eCAL
{
  namespace Config
  {
    struct RegistrationOptions
    {
      public:
        using ms_t = std::chrono::milliseconds;
        RegistrationOptions() = default;
        RegistrationOptions(ms_t reg_timeout_, ms_t reg_refresh_)
        {
          if (reg_refresh_ < reg_timeout_)
          {
            registration_timeout = reg_timeout_;
            registration_refresh = reg_refresh_;
          }
          else
          {
            std::cout << "RegistrationOptions: custom registration refresh >= registration timout. Using default values." << "\n";
          }
        };

        ms_t getTimeout() const { return registration_timeout; }
        ms_t getRefresh() const { return registration_refresh; }

        bool share_ttype = true;
        bool share_tdesc = true;

      private:
        ms_t registration_timeout = ms_t(60000);
        ms_t registration_refresh = ms_t(1000);
    };
  }
}