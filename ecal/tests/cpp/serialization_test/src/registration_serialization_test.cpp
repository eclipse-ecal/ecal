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

#include <serialization/ecal_serialize_sample_registration.h>
#include "registration_generate.h"

#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace eCAL
{
  namespace Registration
  {
    class RegistrationSampleSerializationTest : public ::testing::Test {
    public:

      RegistrationSampleSerializationTest() {
        samples.push_back(GenerateProcessSample());
        samples.push_back(GenerateTopicSample());
        samples.push_back(GenerateServiceSample());
        samples.push_back(GenerateClientSample());
      }

    protected:
      std::vector<Sample> samples;
    };
    using core_cpp_registration_serialization = RegistrationSampleSerializationTest;


    TEST_F(core_cpp_registration_serialization, Registration2String)
    {
      for (const auto& sample_in : samples)
      {
        std::string sample_buffer;
        EXPECT_TRUE(SerializeToBuffer(sample_in, sample_buffer));

        Sample sample_out;
        EXPECT_TRUE(DeserializeFromBuffer(sample_buffer.data(), sample_buffer.size(), sample_out));

        EXPECT_EQ(sample_in, sample_out);
      }
    }

    TEST_F(core_cpp_registration_serialization, Registration2Vector)
    {
      for (const auto& sample_in : samples)
      {
        std::vector<char> sample_buffer;
        EXPECT_TRUE(SerializeToBuffer(sample_in, sample_buffer));

        Sample sample_out;
        EXPECT_TRUE(DeserializeFromBuffer(sample_buffer.data(), sample_buffer.size(), sample_out));

        EXPECT_EQ(sample_in, sample_out);
      }
    }

    TEST_F(core_cpp_registration_serialization, RegistrationList2String)
    {
      SampleList sample_list_in;
      for (const auto& sample_in : samples)
      {
        sample_list_in.push_back(sample_in);
      }

      std::string sample_buffer;
      EXPECT_TRUE(SerializeToBuffer(sample_list_in, sample_buffer));

      SampleList sample_list_out;
      EXPECT_TRUE(DeserializeFromBuffer(sample_buffer.data(), sample_buffer.size(), sample_list_out));

      EXPECT_TRUE(sample_list_in.size() == sample_list_out.size());
      EXPECT_EQ(sample_list_in, sample_list_out);
    }

    TEST_F(core_cpp_registration_serialization, RegistrationList2Vector)
    {
      SampleList sample_list_in;
      for (const auto& sample_in : samples)
      {
        sample_list_in.push_back(sample_in);
      }

      std::vector<char> sample_buffer;
      EXPECT_TRUE(SerializeToBuffer(sample_list_in, sample_buffer));

      SampleList sample_list_out;
      EXPECT_TRUE(DeserializeFromBuffer(sample_buffer.data(), sample_buffer.size(), sample_list_out));

      EXPECT_TRUE(sample_list_in.size() == sample_list_out.size());
      EXPECT_EQ(sample_list_in, sample_list_out);
    }
  }
}
