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
#include <regex>
#include <limits>

namespace eCAL
{
  namespace Config
  {
    /**
     * @brief  Class for evaluation and storing an IP address.
     *
     * @param ip_adress_  The IP address as std::string.
    **/
    class IpAddressV4
    {
    public:      
      IpAddressV4() = default;
      ECAL_API IpAddressV4(const std::string ip_address_);  
             
      const std::string get() { return m_ip_address; }

    private:            
      static bool checkIpString(std::string ip_address_);  

      std::string     m_ip_address{};
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

      int get() { return m_size; };

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