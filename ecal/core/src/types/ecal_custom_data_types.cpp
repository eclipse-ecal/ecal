/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include <stdexcept>
#include <utility>
#include "ecal/types/custom_data_types.h"

#include <asio/ip/address_v4.hpp>

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
      asio::error_code ec{};
      const auto ip = asio::ip::make_address_v4(ip_address_, ec);
      if (ec) {
        throw std::invalid_argument("[IpAddressV4] Invalid IPv4 address: " +
                                    ip_address_);
      }
      if (ip.to_string() != ip_address_) {
        throw std::invalid_argument(
            "[IpAddressV4] IPv4 address: " + ip.to_string() +
            " reinterpreted by the platform as " + ip_address_ +
            " please use the full explicit form");
      }
      m_ip_address = ip_address_;
    }

    const std::string& IpAddressV4::Get() const                         { return m_ip_address; }
    IpAddressV4& IpAddressV4::operator=(const std::string& ip_string_)  { this->validateIpString(ip_string_); return *this; }
    IpAddressV4& IpAddressV4::operator=(const char* ip_string_)         { this->validateIpString(ip_string_); return *this; }
    IpAddressV4::operator const std::string&() const                    { return m_ip_address; }

    bool IpAddressV4::operator==(const eCAL::Types::IpAddressV4& rhs) const        { return m_ip_address == rhs.Get(); }
    bool operator==(eCAL::Types::IpAddressV4 lhs, const char* ip_string_)          { return lhs.Get() == std::string(ip_string_); }
    bool operator==(const char* ip_string_, eCAL::Types::IpAddressV4 rhs)          { return std::move(rhs) == ip_string_; }
    bool operator==(eCAL::Types::IpAddressV4 lhs, const std::string& ip_string_)   { return lhs.Get() == ip_string_; }
    bool operator==(const std::string& ip_string_, eCAL::Types::IpAddressV4 rhs)   { return std::move(rhs) == ip_string_; }
  }  
}
