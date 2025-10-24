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
 * @file   ecal_serialize_sample_payload.h
 * @brief  eCAL sample payload serialization / deserialization
**/

#pragma once

#include "ecal_struct_sample_payload.h"

#include <cstddef>
#include <string>
#include <vector>

namespace eCAL
{
  namespace nanopb
  {
    // payload sample - serialize/deserialize
    bool SerializeToBuffer     (const Payload::Sample& source_sample_, std::vector<char>& target_buffer_);
    bool SerializeToBuffer     (const Payload::Sample& source_sample_, std::string& target_buffer_);
    bool DeserializeFromBuffer (const char* data_, size_t size_, Payload::Sample& target_sample_);
  }

  inline namespace protozero
  {
    // payload sample - serialize/deserialize
    bool SerializeToBuffer     (const Payload::Sample& source_sample_, std::vector<char>& target_buffer_);
    bool SerializeToBuffer     (const Payload::Sample& source_sample_, std::string& target_buffer_);
    bool DeserializeFromBuffer (const char* data_, size_t size_, Payload::Sample& target_sample_);   
  }
}
