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

#include "../../serialization/ecal_serialize_sample_registration.h"

#include <gtest/gtest.h>

namespace eCAL
{
  namespace Registration
  {
    Sample GenerateRegistrationSample();
    bool   CompareRegistrationSamples(const Sample& sample1, const Sample& sample2);

    TEST(core_cpp_serialization, Registration2String)
    {
      Sample sample_in = GenerateRegistrationSample();

      std::string sample_buffer;
      ASSERT_TRUE(SerializeToBuffer(sample_in, sample_buffer));

      Sample sample_out;
      ASSERT_TRUE(DeserializeFromBuffer(sample_buffer.data(), sample_buffer.size(), sample_out));

      ASSERT_TRUE(CompareRegistrationSamples(sample_in, sample_out));
    }

    TEST(core_cpp_serialization, Registration2Vector)
    {
      Sample sample_in = GenerateRegistrationSample();

      std::vector<char> sample_buffer;
      ASSERT_TRUE(SerializeToBuffer(sample_in, sample_buffer));

      Sample sample_out;
      ASSERT_TRUE(DeserializeFromBuffer(sample_buffer.data(), sample_buffer.size(), sample_out));

      ASSERT_TRUE(CompareRegistrationSamples(sample_in, sample_out));
    }

    TEST(core_cpp_serialization, RegistrationList2String)
    {
      SampleList sample_list_in;
      sample_list_in.samples.push_back(GenerateRegistrationSample());
      sample_list_in.samples.push_back(GenerateRegistrationSample());
      sample_list_in.samples.push_back(GenerateRegistrationSample());

      std::string sample_buffer;
      ASSERT_TRUE(SerializeToBuffer(sample_list_in, sample_buffer));

      SampleList sample_list_out;
      ASSERT_TRUE(DeserializeFromBuffer(sample_buffer.data(), sample_buffer.size(), sample_list_out));

      ASSERT_TRUE(sample_list_in.samples.size() == sample_list_out.samples.size());
      ASSERT_TRUE(std::equal(sample_list_in.samples.begin(), sample_list_in.samples.end(), sample_list_out.samples.begin(), CompareRegistrationSamples));
    }

    TEST(core_cpp_serialization, RegistrationList2Vector)
    {
      SampleList sample_list_in;
      sample_list_in.samples.push_back(GenerateRegistrationSample());
      sample_list_in.samples.push_back(GenerateRegistrationSample());
      sample_list_in.samples.push_back(GenerateRegistrationSample());

      std::vector<char> sample_buffer;
      ASSERT_TRUE(SerializeToBuffer(sample_list_in, sample_buffer));

      SampleList sample_list_out;
      ASSERT_TRUE(DeserializeFromBuffer(sample_buffer.data(), sample_buffer.size(), sample_list_out));

      ASSERT_TRUE(sample_list_in.samples.size() == sample_list_out.samples.size());
      ASSERT_TRUE(std::equal(sample_list_in.samples.begin(), sample_list_in.samples.end(), sample_list_out.samples.begin(), CompareRegistrationSamples));
    }
  }
}
