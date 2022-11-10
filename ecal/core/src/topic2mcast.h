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
 * @brief  Create Multicast group based on topic name
**/

#pragma once

#include <sstream>
#include <iostream>
#include <string>
 
namespace eCAL
{
  /**
   * @brief  FNV hash.
  **/
  struct fnv_hash
  {
    size_t operator()( std::string const& s ) const
    {
      size_t result = static_cast<size_t>(2166136261U);
      std::string::const_iterator end = s.end() ;
      for ( std::string::const_iterator iter = s.begin() ;
        iter != end ;
        ++ iter ) {
          result = (16777619 * result)
            ^ static_cast< unsigned char >( *iter ) ;
      }
      return result ;
    }
  };

  inline uint32_t parse_ipv4(const std::string& ipv4_str_)
  {
    uint32_t ipv4 = 0;
    std::stringstream ss(ipv4_str_);
    std::string token;
    int i = 0;
    while(std::getline(ss, token, '.') && (i < 4)) //-V112
    {
      unsigned char mask = static_cast<unsigned char>(atoi(token.c_str()));
      ipv4 = ipv4 << 8 | mask;
      i++;
    }
    return ipv4;
  }

  /**
   * @brief Get multicast address based on the topic_name.
   *
   * @param  tname_       The topic name.
   * @param  mcast_base_  Multicast group base address (e.g. 239.x.x.x).
   * @param  mcast_mask_  Multicast group mask (e.g. x.255.255.255).
   *
   * @return  The multicast address (e.g. "293.151.128.177").
  **/
  inline std::string topic2mcast(const std::string& tname_, const std::string& mcast_base_, const std::string& mcast_mask_)
  {
    struct fnv_hash thash;
    uint32_t hash_v = thash(tname_);

    uint32_t address_mask = parse_ipv4(mcast_mask_);
    uint32_t address_ip = parse_ipv4(mcast_base_) & (~address_mask);
    if ((hash_v & address_mask) < 2)
      address_ip += 2; // avoid .0 or .1 for masked part
    else
      address_ip |= (hash_v & address_mask);

    std::ostringstream address;
    address << (address_ip >> 24) << '.' << ((address_ip & 0x00ffffff) >> 16) << '.'
            << ((address_ip & 0x0000ffff) >> 8) << '.' << (address_ip & 0x000000ff);
    return (address.str());
  }
}
