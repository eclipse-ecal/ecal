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

#include "ecal/types/custom_data_types.h"

#include <array>
#include <regex>
#include <algorithm>
#include <cctype>
#include <ecal_def.h>

namespace{
  const std::regex IPV4_DEC_REGEX = std::regex("^((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])\\.){3}(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])$");
  const std::regex IPV4_HEX_REGEX = std::regex("^([0-9a-fA-F]{1,2}\\.){3}[0-9a-fA-F]{1,2}$");
}

namespace eCAL
{
  namespace Types
  {
    IpAddressV4::IpAddressV4(const std::string& ip_address_)
    {
      validateIpString(ip_address_);
    }

    IpAddressV4::IpAddressV4(const char* ip_address_)
    {
      validateIpString(ip_address_);
    }

    void IpAddressV4::validateIpString(const std::string& ip_address_)
    {
      if (  std::regex_match(ip_address_, IPV4_DEC_REGEX)
         || std::regex_match(ip_address_, IPV4_HEX_REGEX)
      )
      {        
        m_ip_address = ip_address_;
      }
      else
      {
        throw std::invalid_argument("[IpAddressV4] No valid IP address: " + ip_address_);
      }
    }

    const std::string& IpAddressV4::Get() const                         { return m_ip_address; }
    IpAddressV4& IpAddressV4::operator=(const std::string& ip_string_)  { this->validateIpString(ip_string_); return *this; }
    IpAddressV4& IpAddressV4::operator=(const char* ip_string_)         { this->validateIpString(ip_string_); return *this; }
    IpAddressV4::operator const std::string&() const                    { return m_ip_address; }
    
    std::ostream& operator<<(std::ostream& os, const IpAddressV4& ipv4) { os << ipv4.Get(); return os; }

    bool IpAddressV4::operator==(const eCAL::Types::IpAddressV4& rhs) const        { return m_ip_address == rhs.Get(); }
    bool operator==(eCAL::Types::IpAddressV4 lhs, const char* ip_string_)          { return lhs.Get() == std::string(ip_string_); }
    bool operator==(const char* ip_string_, eCAL::Types::IpAddressV4 rhs)          { return rhs == ip_string_; }
    bool operator==(eCAL::Types::IpAddressV4 lhs, const std::string& ip_string_)   { return lhs.Get() == ip_string_; }
    bool operator==(const std::string& ip_string_, eCAL::Types::IpAddressV4 rhs)   { return rhs == ip_string_; }
  }  
}
