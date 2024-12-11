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

#pragma once

#include <serialization/ecal_struct_sample_payload.h>

#include <cstdlib>

namespace eCAL
{
  namespace Payload
  {
    // generate Topic
    TopicInfo GenerateTopic();

    // generate Payload
    Payload GeneratePayload(const char* payload_addr, size_t payload_size);

    // generate Payload
    Payload GeneratePayload(const std::vector<char>& payload_vec);

    // generate Content
    Content GenerateContent(const char* payload_addr, size_t payload_size);
    
    // generate Content
    Content GenerateContent(const std::vector<char>& payload_vec);

    // generate Payload Sample (payload raw pointer + size)
    Sample GeneratePayloadSample(const char* payload_addr, size_t payload_size);

    // generate Payload Sample (payload vector)
    Sample GeneratePayloadSample(const std::vector<char>& payload_vec);
  }
}
