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
 * @brief  Definition of custom data types.
**/

#include "ecal/types/ecal_custom_data_types.h"

namespace eCAL
{
  namespace Config
  {

    // IpAddressV4 definitions

    IpAddressV4::IpAddressV4() = default;

    IpAddressV4::IpAddressV4(const std::string& ip_address_)
    {
      if (checkIpString(ip_address_))
      {
        m_ip_address = ip_address_;
      }
      else
      {
        std::cerr << "[IpAddressV4] No valid IPv4 address: " << ip_address_ << "\n";
        exit(EXIT_FAILURE);
      }
    } 

    bool IpAddressV4::checkIpString(std::string ip_address_)
    {
      if (std::regex_match(ip_address_, std::regex("(([0-9]|[0-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])")))
      {
        return true;
      }
      else if (std::regex_match(ip_address_, std::regex("(([0-9a-fA-F]|[0-9a-fA-F][0-9a-fA-F])\\.){3}([0-9a-fA-F]|[0-9a-fA-F][0-9a-fA-F])")))
      {
        return true;
      }
      else
      {
        return false;
      }
    }
  }
}
