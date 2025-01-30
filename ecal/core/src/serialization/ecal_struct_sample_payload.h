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
 * @file   ecal_struct_sample_payload.h
 * @brief  eCAL payload as struct
**/

#pragma once

#include "ecal_struct_sample_common.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace eCAL
{
  namespace Payload
  {
    // Topic information
    struct TopicInfo
    {
      std::string                         host_name;                        // host name
      uint64_t                            topic_id;                         // topic id
      std::string                         topic_name;                       // topic name
      int32_t                             process_id = 0;                   // process id
    };

    // Topic content payload
    enum ePayloadType
    {
      pl_none = 0,                                                      // payload not initialized
      pl_raw  = 1,                                                      // payload represented as raw pointer + size
      pl_vec  = 2,                                                      // payload represented std::vector<char>
    };

    struct Payload
    {
      ePayloadType                        type     = pl_none;
      const char*                         raw_addr = nullptr;           // payload represented as raw pointer
      size_t                              raw_size = 0;                 //   and its size
      std::vector<char>                   vec;                          // payload represented as std::vector<char>
    };
    
    // Topic content
    struct Content
    {
      int64_t                             id    = 0;                    // payload id
      int64_t                             clock = 0;                    // internal used clock
      int64_t                             time  = 0;                    // time the content was updated
      int64_t                             hash  = 0;                    // unique hash for that payload
      int32_t                             size  = 0;                    // size (additional for none payload "header only samples")
      Payload                             payload;                      // payload represented as raw pointer or a std::vector<char>
    };

    // Payload sample
    struct Sample
    {
      eCmdType                            cmd_type = bct_none;          // payload command type
      TopicInfo                           topic_info;                   // topic information
      Content                             content;                      // topic content
      std::vector<char>                   padding;                      // padding to artificially increase the size of the message. This is a workaround for TCP topics, to get the actual user-payload 8-byte-aligned. REMOVE ME IN ECAL6
    };
  }
}
