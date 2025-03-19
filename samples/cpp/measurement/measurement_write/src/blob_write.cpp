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

#include <ecal/measurement/omeasurement.h>

#include <iostream>

constexpr auto ONE_SECOND = 1000000;

int main(int /*argc*/, char** /*argv*/)
{
  // create a new measurement
  eCAL::measurement::OMeasurement meas(".");

  // create a channel (topic name "blob")
  eCAL::measurement::OChannel blob_channel = meas.Create("blob", eCAL::experimental::measurement::base::DataTypeInformation{});

  // std::string serves as a binary container for data
  // std::vector<std::byte> would be more appropriate if eCAL supported C++17
  std::string binary_data("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8);

  long long timestamp = 0;

  for (int i = 0; i< 100; i++)
  {
    blob_channel << eCAL::measurement::make_frame(binary_data, timestamp);
    timestamp += ONE_SECOND;
  }

  return 0;
}
