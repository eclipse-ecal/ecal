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

#include <ecal/ecal.h>

#include <algorithm>
#include <iostream>

int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "binary_snd");

  // publisher for topic "blob"
  eCAL::CPublisher pub("blob");

  // create binary buffer
  std::vector<unsigned char> bin_buffer(1024);

  // send updates
  unsigned char cnt = 0;
  while (eCAL::Ok())
  {
    // fill buffer
    std::fill(bin_buffer.begin(), bin_buffer.end(), cnt++);

    // send buffer
    pub.Send(bin_buffer.data(), bin_buffer.size());
    std::cout << "Sent buffer filled with " << static_cast<int>(cnt) << std::endl;

    // sleep 100 ms
    eCAL::Process::SleepMS(100);
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
