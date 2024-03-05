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

#include "../../serialization/ecal_serialize_sample_payload.h"

#include <gtest/gtest.h>
#include <random>

namespace
{
  void InitializeVec(std::vector<char>& vec, size_t size)
  {
    // initialize random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);  // Range for char values

    // fill the vector with random values
    vec.resize(size);
    for (char& element : vec)
    {
      element = static_cast<char>(dis(gen));
    }
  }
}

namespace eCAL
{
  namespace Payload
  {
    Sample GeneratePayloadSample(const char* payload_addr, size_t payload_size);
    Sample GeneratePayloadSample(const std::vector<char>& payload_vec);

    bool   ComparePayloadSamples(const Sample& sample1, const Sample& sample2);

    TEST(Serialization, RawPayload2String)
    {
      std::vector<char> payload;
      InitializeVec(payload, 1024);

      Sample sample_in = GeneratePayloadSample(payload.data(), payload.size());

      std::string sample_buffer;
      ASSERT_TRUE(SerializeToBuffer(sample_in, sample_buffer));

      Sample sample_out;
      ASSERT_TRUE(DeserializeFromBuffer(sample_buffer.data(), sample_buffer.size(), sample_out));

      ASSERT_TRUE(ComparePayloadSamples(sample_in, sample_out));
    }

    TEST(Serialization, RawPayload2Vector)
    {
      std::vector<char> payload;
      InitializeVec(payload, 1024);
     
      Sample sample_in = GeneratePayloadSample(payload.data(), payload.size());

      std::vector<char> sample_buffer;
      ASSERT_TRUE(SerializeToBuffer(sample_in, sample_buffer));

      Sample sample_out;
      ASSERT_TRUE(DeserializeFromBuffer(sample_buffer.data(), sample_buffer.size(), sample_out));

      ASSERT_TRUE(ComparePayloadSamples(sample_in, sample_out));
    }

    TEST(Serialization, VecPayload2String)
    {
      std::vector<char> payload;
      InitializeVec(payload, 1024);

      Sample sample_in = GeneratePayloadSample(payload);

      std::string sample_buffer;
      ASSERT_TRUE(SerializeToBuffer(sample_in, sample_buffer));

      Sample sample_out;
      ASSERT_TRUE(DeserializeFromBuffer(sample_buffer.data(), sample_buffer.size(), sample_out));

      ASSERT_TRUE(ComparePayloadSamples(sample_in, sample_out));
    }

    TEST(Serialization, VecPayload2Vector)
    {
      std::vector<char> payload;
      InitializeVec(payload, 1024);

      Sample sample_in = GeneratePayloadSample(payload);

      std::vector<char> sample_buffer;
      ASSERT_TRUE(SerializeToBuffer(sample_in, sample_buffer));

      Sample sample_out;
      ASSERT_TRUE(DeserializeFromBuffer(sample_buffer.data(), sample_buffer.size(), sample_out));

      ASSERT_TRUE(ComparePayloadSamples(sample_in, sample_out));
    }

    TEST(Serialization, VecPayloadEmpty)
    {
      std::vector<char> payload;

      Sample sample_in = GeneratePayloadSample(payload);

      std::vector<char> sample_buffer;
      ASSERT_TRUE(SerializeToBuffer(sample_in, sample_buffer));

      Sample sample_out;
      ASSERT_TRUE(DeserializeFromBuffer(sample_buffer.data(), sample_buffer.size(), sample_out));

      ASSERT_TRUE(ComparePayloadSamples(sample_in, sample_out));
    }

    TEST(Serialization, RawPayloadEmpty)
    {
      Sample sample_in = GeneratePayloadSample(nullptr, 0);

      std::vector<char> sample_buffer;
      ASSERT_TRUE(SerializeToBuffer(sample_in, sample_buffer));

      Sample sample_out;
      ASSERT_TRUE(DeserializeFromBuffer(sample_buffer.data(), sample_buffer.size(), sample_out));

      ASSERT_TRUE(ComparePayloadSamples(sample_in, sample_out));
    }
  }
}
