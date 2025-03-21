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

#include <ecal/msg/string/imeasurement.h>

#include <iostream>

int main(int /*argc*/, char** /*argv*/)
{
  // create a new measurement
  eCAL::measurement::IMeasurement meas(".");

  // Find all channels in the measurement, which have the topicname "hello".
  auto hello_channels = meas.Channels("hello");
  if (hello_channels.size() > 0)
  {
    eCAL::string::IChannel hello_channel{ eCAL::measurement::GetChannel<eCAL::string::IChannel>(meas, *hello_channels.begin()) };

    // iterate over the messages
    for (const auto& hello_entry : hello_channel)
    {
      std::cout << "Hello entry at timestamp " << hello_entry.send_timestamp << std::endl;
      std::cout << hello_entry.message << std::endl;
    }
  }
  return 0;
}
