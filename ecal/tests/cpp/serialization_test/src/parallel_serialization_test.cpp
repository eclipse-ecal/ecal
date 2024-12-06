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

#include "serialization/ecal_serialize_logging.h"
#include "serialization/ecal_serialize_sample_registration.h"

#include "registration_generate.h"
#include "logging_generate.h"

#include <gtest/gtest.h>
#include <string>
#include <thread>


namespace eCAL
{
    void SerializeLogMessage()
    {
        Logging::SLogMessage message_in = Logging::GenerateLogMessage();

        for (int i = 0; i < 10000; ++i)
        {
        std::string message_buffer;
        SerializeToBuffer(message_in, message_buffer);

        Logging::SLogMessage message_out;
        DeserializeFromBuffer(message_buffer.data(), message_buffer.size(), message_out);
        }
    }

      void SerializePayloadMessage()
      {
          Registration::Sample message_in = Registration::GenerateTopicSample();

          for (int i = 0; i < 10000; ++i)
          {
              std::string message_buffer;
              SerializeToBuffer(message_in, message_buffer);

              Registration::Sample message_out;
              DeserializeFromBuffer(message_buffer.data(), message_buffer.size(), message_out);
          }
      }


      TEST(core_cpp_serialization, ParallelSerialization)
      {
          std::thread logging_serializer_thread(SerializeLogMessage);
          std::thread sample_serializer_thread(SerializePayloadMessage);

          logging_serializer_thread.join();
          sample_serializer_thread.join();
  }

}
