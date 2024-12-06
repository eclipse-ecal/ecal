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

#include <algorithm>
#include <serialization/ecal_serialize_logging.h>

#include "logging_generate.h"
#include "logging_compare.h"

#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace eCAL
{
  namespace Logging
  {
    TEST(core_cpp_serialization, LogMessage2String)
    {
      SLogMessage message_in = GenerateLogMessage();

      std::string message_buffer;
      ASSERT_TRUE(SerializeToBuffer(message_in, message_buffer));

      SLogMessage message_out;
      ASSERT_TRUE(DeserializeFromBuffer(message_buffer.data(), message_buffer.size(), message_out));

      ASSERT_TRUE(CompareLogMessages(message_in, message_out));
    }

    TEST(core_cpp_serialization, LogMessage2Vector)
    {
      SLogMessage message_in = GenerateLogMessage();

      std::vector<char> message_buffer;
      ASSERT_TRUE(SerializeToBuffer(message_in, message_buffer));

      SLogMessage message_out;
      ASSERT_TRUE(DeserializeFromBuffer(message_buffer.data(), message_buffer.size(), message_out));

      ASSERT_TRUE(CompareLogMessages(message_in, message_out));
    }


    TEST(core_cpp_serialization, LogMessageList2String)
    {
      SLogging message_list_in;
      message_list_in.log_messages.push_back(GenerateLogMessage());
      message_list_in.log_messages.push_back(GenerateLogMessage());
      message_list_in.log_messages.push_back(GenerateLogMessage());

      std::string sample_buffer;
      ASSERT_TRUE(SerializeToBuffer(message_list_in, sample_buffer));

      SLogging message_list_out;
      ASSERT_TRUE(DeserializeFromBuffer(sample_buffer.data(), sample_buffer.size(), message_list_out));

      ASSERT_TRUE(message_list_in.log_messages.size() == message_list_out.log_messages.size());
      ASSERT_TRUE(std::equal(message_list_in.log_messages.begin(), message_list_in.log_messages.end(), message_list_out.log_messages.begin(), CompareLogMessages));
    }

    TEST(core_cpp_serialization, LogMessageList2Vector)
    {
      SLogging message_list_in;
      message_list_in.log_messages.push_back(GenerateLogMessage());
      message_list_in.log_messages.push_back(GenerateLogMessage());
      message_list_in.log_messages.push_back(GenerateLogMessage());

      std::vector<char> sample_buffer;
      ASSERT_TRUE(SerializeToBuffer(message_list_in, sample_buffer));

      SLogging message_list_out;
      ASSERT_TRUE(DeserializeFromBuffer(sample_buffer.data(), sample_buffer.size(), message_list_out));

      ASSERT_TRUE(message_list_in.log_messages.size() == message_list_out.log_messages.size());
      ASSERT_TRUE(std::equal(message_list_in.log_messages.begin(), message_list_in.log_messages.end(), message_list_out.log_messages.begin(), CompareLogMessages));
    }
  }
}
