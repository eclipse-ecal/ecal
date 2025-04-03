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

#include "payload_compare.h"

#include <serialization/ecal_struct_sample_payload.h>
#include <vector>

namespace eCAL
{
  namespace Payload
  {
    // compare two samples for equality
    bool ComparePayloadSamples(const Sample& sample1, const Sample& sample2)
    {
      // compare cmd_type
      if (sample1.cmd_type != sample2.cmd_type) {
        return false;
      }

      // compare topic info
      if (sample1.topic_info.host_name  != sample2.topic_info.host_name ||
          sample1.topic_info.process_id != sample2.topic_info.process_id ||
          sample1.topic_info.topic_id   != sample2.topic_info.topic_id ||
          sample1.topic_info.topic_name != sample2.topic_info.topic_name) {
        return false;
      }

      // compare content
      if (sample1.content.id    != sample2.content.id ||
          sample1.content.clock != sample2.content.clock ||
          sample1.content.time  != sample2.content.time ||
          sample1.content.hash  != sample2.content.hash ||
          sample1.content.size  != sample2.content.size) {
        return false;
      }

      // deserialized payloads (sample2) must always use payload type 'pl_vec'
      if (sample2.content.payload.type != pl_vec) {
        return false;
      }

      if ((sample1.content.payload.type != pl_none) &&
          (sample2.content.payload.type != pl_none))
      {
        // extract payload1
        std::vector<char> payload1_vec;
        switch (sample1.content.payload.type)
        {
        case pl_none:
          break;
        case pl_raw:
          payload1_vec = std::vector<char>(sample1.content.payload.raw_addr, sample1.content.payload.raw_addr + sample1.content.payload.raw_size);
          break;
        case pl_vec:
          payload1_vec = sample1.content.payload.vec;
          break;
        }

        // compare payloads
        if (payload1_vec != sample2.content.payload.vec) {
          return false;
        }
      }

      // compare padding
      if (sample1.padding != sample2.padding) {
        return false;
      }

      // all comparisons passed, samples are equal
      return true;
    }
  }
}
