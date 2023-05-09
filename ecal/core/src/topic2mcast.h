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

#include <cstdint>
#include <sstream>
#include <iostream>
#include <string>
 
namespace eCAL
{
  namespace UDP
  {
    /**
     * @brief  FNV hash.
    **/
    struct fnv_hash
    {
      size_t operator()(std::string const& s) const
      {
        size_t result = static_cast<size_t>(2166136261U);
        std::string::const_iterator end = s.end();
        for (std::string::const_iterator iter = s.begin();
          iter != end;
          ++iter) {
          result = (16777619 * result)
            ^ static_cast<unsigned char>(*iter);
        }
        return result;
      }
    };

    namespace V2 {

      inline uint32_t parse_ipv4(const std::string& ipv4_str_)
      {
        uint32_t ipv4 = 0;
        std::stringstream ss(ipv4_str_);
        std::string token;
        int i = 0;
        while (std::getline(ss, token, '.') && (i < 4)) //-V112
        {
          unsigned char mask = static_cast<unsigned char>(atoi(token.c_str()));
          ipv4 = ipv4 << 8 | mask;
          i++;
        }
        return ipv4;
      }

      inline std::string serialize_ipv4(uint32_t ipv4)
      {
        std::ostringstream address;
        address << (ipv4 >> 24) << '.' << ((ipv4 & 0x00ffffff) >> 16) << '.'
          << ((ipv4 & 0x0000ffff) >> 8) << '.' << (ipv4 & 0x000000ff);
        return address.str();
      }

      /**
      * combines an ip address and a hash with a mask, that determines the to be used MC adress
      * e.g. samples adress:           0xFFAAAAAA
      *              hash:             0x6789ABCD
      *              mask:             0x00FFFFFF
      *              expected result:  0xFF89ABCD
      *
      **/
      inline uint32_t combine_ip_and_hash_with_mask(uint32_t address, uint32_t hash, uint32_t mask)
      {
        return (address & mask) | (hash & ~mask);
      }

      inline uint32_t increase_adress_by_one(uint32_t address, uint32_t mask)
      {
        return (address & mask) | ((address + 1) & ~mask);
      }

      inline std::string topic2mcast_hash(uint32_t hash_v, const std::string& mcast_base_, const std::string& mcast_mask_)
      {
        uint32_t address_mask = parse_ipv4(mcast_mask_);
        uint32_t address_ip = parse_ipv4(mcast_base_);

        auto mcast{ combine_ip_and_hash_with_mask(address_ip, hash_v, address_mask) };

        // avoid collision on mcast_base
        // we don't want to publish a topic on the same adress as registration information
        if (mcast == address_ip)
          mcast = increase_adress_by_one(address_ip, address_mask);

        return serialize_ipv4(mcast);

      }

      /**
       * @brief Get multicast address based on the topic_name.
       *
       * @param  tname_       The topic name.
       * @param  mcast_base_  Multicast group base address (e.g. 239.x.x.x).
       * @param  mcast_mask_  Multicast group mask (e.g. x.255.255.255).
       *
       * @return  The multicast address (e.g. "239.151.128.177").
      **/
      inline std::string topic2mcast(const std::string& tname_, const std::string& mcast_base_, const std::string& mcast_mask_)
      {
        struct fnv_hash thash;
        uint32_t hash_v = static_cast<uint32_t>(thash(tname_));

        return topic2mcast_hash(hash_v, mcast_base_, mcast_mask_);
      }

    }

    namespace V1
    {
      // We split it in two functions for better testing
      // the content is the same!!!
      inline std::string topic2mcast_hash(size_t hash_v, const std::string& mcast_base_, const std::string& mcast_mask_)
      {
        unsigned char address_mask[4] = { 0, 0xFF, 0xFF, 0xFF }; //-V112
        std::stringstream ss(mcast_mask_);
        std::string token;
        int i = 0;
        while (std::getline(ss, token, '.') && (i < 4)) //-V112
        {
          unsigned char mask = static_cast<unsigned char>(atoi(token.c_str()));
          address_mask[i] = mask;
          i++;
        }

        unsigned char address_ip[4] = { 0 }; //-V112
        address_ip[1] = static_cast<unsigned char>((hash_v >> 16) & address_mask[1]);
        address_ip[2] = static_cast<unsigned char>((hash_v >> 8) & address_mask[2]);
        address_ip[3] = static_cast<unsigned char>((hash_v)&address_mask[3]);
        if (address_ip[3] < 2) address_ip[3] = 2; // avoid 0.0.0 and 0.0.1

        std::string address = mcast_base_.substr(0, 3);
        for (auto t = 1; t < 4; ++t) //-V112
        {
          address += ".";
          address += std::to_string(address_ip[t]);
        }
        return(address);
      }

      /**
      * @brief This is the legacy way to compute the mcast address based on the topic name
      *        It behaves incorrectly (meaning not what the user would expect)
      *        It is kept here for compatibility purposes (can be activated in the ini file)
      *
      * @param  tname_       The topic name.
      * @param  mcast_base_  Multicast group base address (e.g. 239.x.x.x).
      * @param  mcast_mask_  Multicast group mask (e.g. x.255.255.255).
      *
      * @return  The multicast address (e.g. "239.151.128.177").
      **/
      inline std::string topic2mcast(const std::string& tname_, const std::string& mcast_base_, const std::string& mcast_mask_)
      {
        struct fnv_hash thash;
        size_t hash_v = thash(tname_);

        return topic2mcast_hash(hash_v, mcast_base_, mcast_mask_);
      }
    }
  

  }
}
