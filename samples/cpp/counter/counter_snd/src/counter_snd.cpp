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

#include <iostream>
#include <sstream>
#include <chrono>
#include <vector>

int main(int argc, char **argv)
{
  size_t payload_size(10240);
  if(argc > 1) payload_size = atoi(argv[1]);
  if(payload_size < 1) payload_size = 1;

  // initialize eCAL API
  eCAL::Initialize(argc, argv, "counter_snd");

  // create publisher for topic "Counter"
  eCAL::CPublisher pub("Counter", "long long");

  // timer
  std::chrono::steady_clock::time_point start_time(std::chrono::nanoseconds(0));
  long long                             clock (0);
  int                                   msgs  (0);
  unsigned long long                    bytes (0);
  size_t                                slen  (0);

  // default send string
  std::vector<char> send_a(payload_size);

  std::cout << "Message size  =  " << int(payload_size) << " Byte = " << int(payload_size/1024) << " kByte = " << int(payload_size/1024/1024) << " MByte" << std::endl << std::endl;
  slen = payload_size;

  // send updates
  start_time = std::chrono::steady_clock::now();
  while(eCAL::Ok())
  {
    // send content
    pub.Send(send_a.data(), payload_size);

    // collect data
    clock++;
    msgs++;
    bytes += slen;

    // put the clock into the send buffer
    ((long long*)send_a.data())[0] = clock;

    // check timer and print results every second
    if(clock%10000 == 0)
    {
      std::chrono::duration<double> diff_time = std::chrono::steady_clock::now() - start_time;
      if (diff_time >= std::chrono::duration<double>(1.0))
      {
        start_time = std::chrono::steady_clock::now();
        std::stringstream out;
        out << "Message size (kByte):  " << int(slen / 1024)                             << std::endl;
        out << "kByte/s:               " << int(bytes / 1024 / diff_time.count())        << std::endl;
        out << "MByte/s:               " << int(bytes / 1024 / 1024 / diff_time.count()) << std::endl;
        out << "Messages/s:            " << int(msgs / diff_time.count())                << std::endl;
        out << "Clock:                 " << clock                                        << std::endl;
        std::cout << out.str() << std::endl;
        msgs  = 0;
        bytes = 0;

        eCAL::Logging::Log(out.str());
      }
    }
  }

  // destroy publisher
  pub.Destroy();

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
