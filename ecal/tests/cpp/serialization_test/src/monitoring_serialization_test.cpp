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

#include <serialization/ecal_serialize_monitoring.h>
#include "monitoring_generate.h"
#include "monitoring_compare.h"

#include <gtest/gtest.h>
#include <vector>

namespace eCAL
{
  namespace Monitoring
  {
    TEST(core_cpp_serialization, Monitoring2String)
    {
      SMonitoring message_in = GenerateMonitoring();

      std::string message_buffer;
      ASSERT_TRUE(SerializeToBuffer(message_in, message_buffer));

      SMonitoring message_out;
      ASSERT_TRUE(DeserializeFromBuffer(message_buffer.data(), message_buffer.size(), message_out));

      ASSERT_TRUE(CompareMonitorings(message_in, message_out));
    }

    TEST(core_cpp_serialization, Monitoring2Vector)
    {
      SMonitoring message_in = GenerateMonitoring();

      std::vector<char> message_buffer;
      ASSERT_TRUE(SerializeToBuffer(message_in, message_buffer));

      SMonitoring message_out;
      ASSERT_TRUE(DeserializeFromBuffer(message_buffer.data(), message_buffer.size(), message_out));

      ASSERT_TRUE(CompareMonitorings(message_in, message_out));
    }
  }
}
