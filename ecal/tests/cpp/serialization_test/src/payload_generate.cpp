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

#include "payload_generate.h"
#include "common_generate.h"
#include <serialization/ecal_struct_sample_payload.h>

#include <cstdlib>
#include <vector>

namespace eCAL
{
  namespace Payload
  {
    // generate Topic
    TopicInfo GenerateTopic()
    {
      TopicInfo topic;
      topic.host_name  = GenerateString(8);
      topic.process_id = rand() % 1000;
      topic.topic_id   = rand();
      topic.topic_name = GenerateString(10);

      return topic;
    }

    // generate Payload
    Payload GeneratePayload(const char* payload_addr, size_t payload_size)
    {
      Payload payload;
      payload.type     = pl_raw;
      payload.raw_addr = payload_addr;
      payload.raw_size = payload_size;

      return payload;
    }

    // generate Payload
    Payload GeneratePayload(const std::vector<char>& payload_vec)
    {
      Payload payload;
      payload.type = pl_vec;
      payload.vec  = payload_vec;

      return payload;
    }

    // generate Content
    Content GenerateContent(const char* payload_addr, size_t payload_size)
    {
      Content content;
      content.id      = rand() % 100;
      content.clock   = rand() % 1000;
      content.time    = rand() % 10000;
      content.hash    = rand() % 100000;
      content.size    = rand() % 50;
      content.payload = GeneratePayload(payload_addr, payload_size);

      return content;
    }

    // generate Content
    Content GenerateContent(const std::vector<char>& payload_vec)
    {
      Content content;
      content.id      = rand() % 100;
      content.clock   = rand() % 1000;
      content.time    = rand() % 10000;
      content.hash    = rand() % 100000;
      content.size    = rand() % 50;
      content.payload = GeneratePayload(payload_vec);

      return content;
    }

    // generate Payload Sample (payload raw pointer + size)
    Sample GeneratePayloadSample(const char* payload_addr, size_t payload_size)
    {
      Sample sample;
      sample.cmd_type   = static_cast<eCmdType>(rand() % 17);  //  command type
      sample.topic_info = GenerateTopic();
      sample.content    = GenerateContent(payload_addr, payload_size);

      return sample;
    }

    // generate Payload Sample (payload vector)
    Sample GeneratePayloadSample(const std::vector<char>& payload_vec)
    {
      Sample sample;
      sample.cmd_type   = static_cast<eCmdType>(rand() % 17);  //  command type
      sample.topic_info = GenerateTopic();
      sample.content    = GenerateContent(payload_vec);
      sample.padding    = GenerateRandomVector(8);

      return sample;
    }
  }
}
