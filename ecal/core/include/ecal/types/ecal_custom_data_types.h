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
 * @file   ecal_custom_types.h
 * @brief  eCAL custom types for configuration declarations
**/

#pragma once

#include "ecal/ecal_os.h"

#include <string>
#include <iostream>
#include <limits>

namespace eCAL
{
  namespace Config
  {
    /**
     * @brief  Class for evaluation and storing an IPv4/IPv6 address.
     *         Invalid addresses: 255.255.255.255, 127.0.0.1, 0.0.0.0
     *
     * @param ip_adress_  The IP address as std::string.
    **/
    class IpAddressV4
    {
    public:      
      ECAL_API IpAddressV4();
      ECAL_API IpAddressV4(const std::string& ip_address_);  
             
      std::string get() const { return m_ip_address; }
      
      void operator=(IpAddressV4& other)       { this->m_ip_address = other.get(); };
      void operator=(const std::string& other) { this->validateIpString(other); };

    private:            
      ECAL_API void validateIpString(const std::string& ip_address_); 
      void exitApp(const std::string& ip_adress_ = std::string("")); 

      std::string m_ip_address;
    };

    /**
     * @brief  Template class to specify sizes with a concrete minimum, maximum and step size definition.
     *
     * @tparam MIN   Optional minimum possible size. Default: 0
     * @tparam STEP  Optional step size.             Default: 1 
     * @tparam MAX   Optional maximum possible size. Default: std::numeric_limits<int>::max()
     * 
     * @param size_  Optional size value. If not set, LimitSize::get() will return the MIN value.
    **/
    template<int MIN = 0, int STEP = 1, int MAX = std::numeric_limits<int>::max()>
    class LimitSize
    {
    public:
      LimitSize(int size_ = MIN)
      {
        if (size_ >= MIN && size_ <= MAX && size_ % STEP == 0)
        {
          m_size = size_;
        }
        else
        {
          std::cerr << "[LimitSize] Faulty size configuration or assignment. MIN: " << MIN << " MAX: " << MAX << " STEP: " << STEP << " VALUE:" << size_ << "\n";
          exit(EXIT_FAILURE);
        }
      };

      int get() const { return m_size; };
      
      bool operator==(const LimitSize& other) const { return this->m_size == other.get(); };
      void operator=(const LimitSize& other) { this->m_size = other.get(); };
      friend bool operator==(const LimitSize& lhs, const int& rhs) { return lhs.get() == rhs; };
      friend bool operator==(const int& lhs, const LimitSize& rhs) { return rhs == lhs; };
      friend bool operator==(const LimitSize& lhs, const unsigned int& rhs) { return lhs.get() == static_cast<int>(rhs); };
      friend bool operator==(const unsigned int& lhs, const LimitSize& rhs) { return rhs == lhs; };
      
    private:
      int m_size;
    };

    enum class UdpConfigVersion
    {
      V1 = 1,
      V2 = 2
    };
  }
}        