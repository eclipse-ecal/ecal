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

#include <string>
#include <iostream>
#include <regex>

namespace eCAL
{
  namespace Config
  {
    class IpAddressV4
    {
    public:
      IpAddressV4() = default;
      IpAddressV4(const std::string ip_address_)
      {
        if (checkIpString(ip_address_))
        {
          m_ip_address = ip_address_;
        }
        else
        {
          std::cout << "IpAddressV4 error: check your IpAddress settings." << std::endl;
        }
      }            

      const std::string get() { return m_ip_address; }

    private:            
      static bool checkIpString(std::string ip_address_)
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
        
      std::string     m_ip_address;
    };

    template<int MIN = 0, int STEP = 1, long long MAX = std::numeric_limits<int>::max()>
    class LimitSize
    {
    public:
      LimitSize(int size_ = MIN)
      {
        if (size_ >= m_size_min && size_ <= m_size_max && size_ % m_size_step == 0)
        {
          m_size = size_;
        }
        else
        {
          std::cout << "LimitSize: faulty size configuration or assignment - using minimum size " << m_size_min << std::endl;
        }
      };

      int get() { return m_size; };

    private:
      int       m_size_min  = MIN;
      long long m_size_max  = MAX;
      int       m_size_step = STEP;

      int m_size = MIN;
    };

    enum class UdpConfigVersion
    {
      V1 = 1,
      V2 = 2
    };
  }
}        