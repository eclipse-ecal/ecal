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
 * @file   types/custom_types.h
 * @brief  eCAL custom types for configuration declarations
**/

#pragma once

#include "ecal/os.h"

#include <string>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace eCAL
{
  namespace Types
  {
    /**
     * @brief  Class for evaluation and storing an IPv4/IPv6 address.
     *         Invalid addresses: 255.255.255.255, 127.0.0.1, 0.0.0.0
     *
     * @param ip_address_  The IP address as std::string.
     * 
     * @throws std::invalid_argument exception.
    **/
    class IpAddressV4
    {
    public:
      ECAL_API IpAddressV4(const std::string& ip_address_);
      ECAL_API IpAddressV4(const char* ip_address_);

      ECAL_API std::string Get() const;

      ECAL_API IpAddressV4& operator=(const std::string& ip_string_);
      ECAL_API IpAddressV4& operator=(const char* ip_string_);
      ECAL_API operator std::string() const;
      ECAL_API bool operator==(const eCAL::Types::IpAddressV4& rhs) const;  
      ECAL_API friend bool operator==(eCAL::Types::IpAddressV4 lhs, const char* ip_string_);
      ECAL_API friend bool operator==(const char* ip_string_, eCAL::Types::IpAddressV4 rhs);
      ECAL_API friend bool operator==(eCAL::Types::IpAddressV4 lhs, const std::string& ip_string_);
      ECAL_API friend bool operator==(const std::string& ip_string_, eCAL::Types::IpAddressV4 rhs);

    private:            
      ECAL_API void validateIpString(const std::string& ip_address_);

      std::string m_ip_address{};
    };

    enum class UdpConfigVersion
    {
      V1 = 1,
      V2 = 2
    };

    enum class UDPMode
    {
      NETWORK,
      LOCAL
    };

  }
} 