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

#include <ecal/measurement/imeasurement.h>

#include <iostream>

int main(int /*argc*/, char** /*argv*/)
{
  // create a new measurement
  eCAL::measurement::IMeasurement meas(MEASUREMENT_PATH);

  // create a channel (topic name "blob")
  auto blob_channels = meas.Channels("blob");
  if (blob_channels.size() > 0)
  {
    eCAL::measurement::IChannel blob_channel = meas.Get(*blob_channels.begin());

    // iterate over the messages
    for (const auto& blob_entry : blob_channel)
    {
      std::cout << "Person object at timestamp " << blob_entry.send_timestamp << std::endl;
    }
  }
  return 0;
}
