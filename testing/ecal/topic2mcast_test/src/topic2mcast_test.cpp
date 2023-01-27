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

#include <ecal/ecal.h>
#include "topic2mcast.h"

#include <string>
#include <chrono>
#include <thread>
#include <cstdint>

#include <gtest/gtest.h>

#include <iostream>


struct IP
{
  uint32_t int_ip;
  std::string string_ip;
};

std::vector<IP> ipv4_int_string_vector
{
  {0, "0.0.0.0"},
  {0xFFFFFFFF,  "255.255.255.255"},
  {0xEF000001,  "239.0.0.1"},
  {0x0000000F,  "0.0.0.15"},
  {0xC88A00FA,  "200.138.0.250"},
};

struct IPHashMaskResult
{
  uint32_t ip;
  uint32_t hash;
  uint32_t mask;
  uint32_t result;
};

std::vector<IPHashMaskResult> ip_hash_mask_result_vector
{                                                     // multicast group  computed hash    mutlicast mask   computed mcast address
  {0xFFAAAAAA, 0x6789ABCD, 0xFF000000, 0xFF89ABCD },  // 255.170.170.170, 103.137.171.205, 255.0.0.0,       255.137.171.205
  {0xFFAAAAAA, 0x6789ABCD, 0xFFFF0000, 0xFFAAABCD },  // 255.170.170.170, 103.137.171.205, 255.255.0.0,     255.170.171.205
  {0xFFAAAAAA, 0x6789ABCD, 0xFFFFFF00, 0xFFAAAACD },  // 255.170.170.170, 103.137.171.205, 255.255.255.0,   255.170.170.205
  {0xFFAAAAAA, 0x6789ABCD, 0xFFFFFFF0, 0xFFAAAAAD },  // 255.170.170.170, 103.137.171.205, 255.255.255.240, 255.170.170.173
  {0xFFAAAAAA, 0x6789ABCD, 0xFFFFFFFF, 0xFFAAAAAA },
  {0xFFAAAAAA, 0x0000AA00, 0xFF000000, 0xFF00AA00 }
};

struct IpMaskResult
{
  uint32_t ip;
  uint32_t mask;
  uint32_t result;
};

// We don't want to collide on addresses. thus the calculated mcast address needs to be increased by one if collision occurs
// however, we need to wrap according to the mask.
std::vector<IpMaskResult> increase_by_one_vector
{
  {0xFFAAAAAA, 0xFF000000, 0xFFAAAAAB },  // 255.170.170.170, 255.0.0.0, 255.170.170.171
  {0xABFFFFFF, 0xFF000000, 0xAB000000 },  // 171.255.255.255, 255.0.0.0, 171.0.0.0         
  {0xABFFFFFF, 0xFFFF0000, 0xABFF0000 },  
  {0xABFFFFFF, 0xFFFFFF00, 0xABFFFF00 }, 
  {0xABFFFFFF, 0xFFFFFF00, 0xABFFFF00 },
  {0xABFFFFFF, 0xFFFFFFF0, 0xABFFFFF0 },
  {0xABFFFFFF, 0xFFFFFFFF, 0xABFFFFFF },
  {0xABFFFFAB, 0xFFFFFFFF, 0xABFFFFAB }
};



struct Topic2MCastLegacy
{
  size_t hash;
  std::string ip;
  std::string mask;
  std::string result;
};

std::vector<Topic2MCastLegacy> v1_behavior_vector
{
  {0xAAAAAA00, "239.0.0.1", "0.0.0.15", "239.0.0.2" },  
  {0xAAAAAA01, "239.0.0.1", "0.0.0.15", "239.0.0.2" },  
  {0xAAAAAA02, "239.0.0.1", "0.0.0.15", "239.0.0.2" },  
  {0xAAAAAA03, "239.0.0.1", "0.0.0.15", "239.0.0.3" },  
  {0xAAAAAA04, "239.0.0.1", "0.0.0.15", "239.0.0.4" },
  {0xFFAAAAA5, "239.0.0.1", "0.0.0.15", "239.0.0.5" },
  {0xFFAAAAA5, "239.100.100.1", "0.0.0.15", "239.0.0.5" }
};

std::vector<Topic2MCastLegacy> v2_behavior_vector
{
  {0xAAAAAA00, "239.0.0.1", "255.255.255.170", "239.0.0.0" },
  {0xAAAAAA01, "239.0.0.1", "255.255.255.170", "239.0.0.2" },
  {0xAAAAAA02, "239.0.0.1", "255.255.255.170", "239.0.0.2" },
  {0xAAAAAA03, "239.0.0.1", "255.255.255.170", "239.0.0.3" },
  {0xAAAAAA04, "239.0.0.1", "255.255.255.170", "239.0.0.4" },
  {0xFFAAAAA5, "239.0.0.1", "255.255.255.170", "239.0.0.5" },
  {0xFFAAAAAF, "239.100.100.15", "255.255.255.170", "239.100.100.0" }
};

struct Topic2McastInput
{
  std::string multicast_group;
  std::string mulitcast_mask;
  std::string topic_name;
};

std::vector<std::tuple<Topic2McastInput, std::string, std::string>> test;


TEST(Topic2Mcast, ParseIPV4)
{ 
   for (const auto& ip : ipv4_int_string_vector)
   {
     EXPECT_EQ(eCAL::UDP::V2::parse_ipv4(ip.string_ip), ip.int_ip);
   }
}

TEST(Topic2Mcast, SerializeIPV4)
{
  for (const auto& ip : ipv4_int_string_vector)
  {
    EXPECT_EQ(eCAL::UDP::V2::serialize_ipv4(ip.int_ip), ip.string_ip);
  }
}

TEST(Topic2Mcast, CombineIpAndHashWithMask)
{
  for (const auto& object : ip_hash_mask_result_vector)
  {
    EXPECT_EQ(eCAL::UDP::V2::combine_ip_and_hash_with_mask(object.ip, object.hash, object.mask), object.result);
  } 
}

TEST(Topic2Mcast, IncreaseByOne)
{
  for (const auto& object : increase_by_one_vector)
  {
    EXPECT_EQ(eCAL::UDP::V2::increase_adress_by_one(object.ip, object.mask), object.result);
  }
}

TEST(Topic2Mcast, LegacyBehavior)
{
  for (const auto& object : v1_behavior_vector)
  {
    EXPECT_EQ(eCAL::UDP::V1::topic2mcast_hash(object.hash, object.ip, object.mask), object.result);
  }
}

