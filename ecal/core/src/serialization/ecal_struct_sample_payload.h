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
 * @file   ecal_struct_sample_payload.h
 * @brief  eCAL payload as struct
**/

#pragma once

#include "ecal_struct_sample_common.h"

#include <map>
#include <string>
#include <vector>

namespace eCAL
{
  namespace Payload
  {
    // Topic information
    struct Topic
    {
      std::string                         hname;                        // host name
      std::string                         tid;                          // topic id
      std::string                         tname;                        // topic name
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
      Topic                               topic;                        // topic information
      Content                             content;                      // topic content
    };
  }
}
