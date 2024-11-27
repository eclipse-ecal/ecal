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

#include <serialization/ecal_serialize_service.h>
#include "service_compare.h"
#include "service_generate.h"

#include <gtest/gtest.h>
#include <string>

namespace eCAL
{
  namespace Service
  {
    TEST(core_cpp_serialization, Request2String)
    {
      Request sample_in = GenerateRequest();

      std::string sample_buffer;
      ASSERT_TRUE(SerializeToBuffer(sample_in, sample_buffer));

      Request sample_out;
      ASSERT_TRUE(DeserializeFromBuffer(sample_buffer.data(), sample_buffer.size(), sample_out));

      ASSERT_TRUE(CompareRequests(sample_in, sample_out));
    }

    TEST(core_cpp_serialization, Request2Vector)
    {
      Request sample_in = GenerateRequest();

      std::vector<char> sample_buffer;
      ASSERT_TRUE(SerializeToBuffer(sample_in, sample_buffer));

      Request sample_out;
      ASSERT_TRUE(DeserializeFromBuffer(sample_buffer.data(), sample_buffer.size(), sample_out));

      ASSERT_TRUE(CompareRequests(sample_in, sample_out));
    }

    TEST(core_cpp_serialization, Response2String)
    {
      Response sample_in = GenerateResponse();

      std::string sample_buffer;
      ASSERT_TRUE(SerializeToBuffer(sample_in, sample_buffer));

      Response sample_out;
      ASSERT_TRUE(DeserializeFromBuffer(sample_buffer.data(), sample_buffer.size(), sample_out));

      ASSERT_TRUE(CompareResponses(sample_in, sample_out));
    }

    TEST(core_cpp_serialization, Response2Vector)
    {
      Response sample_in = GenerateResponse();

      std::vector<char> sample_buffer;
      ASSERT_TRUE(SerializeToBuffer(sample_in, sample_buffer));

      Response sample_out;
      ASSERT_TRUE(DeserializeFromBuffer(sample_buffer.data(), sample_buffer.size(), sample_out));

      ASSERT_TRUE(CompareResponses(sample_in, sample_out));
    }
  }
}
