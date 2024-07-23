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
#include <ecal/ecal_publisher.h>

#include <chrono>
#include <iostream>
#include <sstream>

#include "binary_payload_writer.h"

// performance settings
const bool   zero_copy              (true);
const int    buffer_count           (1);
const int    acknowledge_timeout_ms (50);
const size_t payload_size_default   (8* 1024 * 1024);

// main entry
int main(int argc, char **argv)
{
  size_t payload_size(payload_size_default);
  if(argc > 1) payload_size = atoi(argv[1]);
  if(payload_size < 1) payload_size = 1;

  // initialize eCAL API
  eCAL::Initialize(argc, argv, "performance_snd");

  // create payload
  CBinaryPayload binary_payload(payload_size);

  // create publisher config
  eCAL::Publisher::Configuration pub_config;

  // enable zero copy mode
  std::cout << "Zero copy mode: " << zero_copy << std::endl;
  pub_config.layer.shm.zero_copy_mode = zero_copy;
  
  // set write buffer count
  std::cout << "Number of write buffers: " << buffer_count << std::endl;
  pub_config.layer.shm.memfile_buffer_count = buffer_count;

  // enable handshake mode
  std::cout << "Acknowledge timeout: " << acknowledge_timeout_ms << " ms" << std::endl;
  pub_config.layer.shm.acknowledge_timeout_ms = acknowledge_timeout_ms;
  std::cout << std::endl;

  // create publisher
  eCAL::CPublisher pub("Performance", pub_config);
 
  // counter
  long long msgs (0);
  long long bytes(0);
  long long clock(0);

  // set start time
  auto start_time = std::chrono::steady_clock::now();

  // send updates
  while(eCAL::Ok())
  {
    // send content
    pub.Send(binary_payload);

    // manage counters
    clock++;
    msgs++;
    bytes += payload_size;

    // check timer and print results every second
    if(clock%2000 == 0)
    {
      const std::chrono::duration<double> diff_time = std::chrono::steady_clock::now() - start_time;
      if(diff_time >= std::chrono::seconds(1))
      {
        // log results
        std::stringstream out;
        out << "Message size (kByte):  " << (unsigned int)(binary_payload.GetSize() / 1024)                      << std::endl;
        out << "kByte/s:               " << (unsigned int)(bytes / 1024.0 /                   diff_time.count()) << std::endl;
        out << "MByte/s:               " << (unsigned int)(bytes / 1024.0 / 1024.0 /          diff_time.count()) << std::endl;
        out << "GByte/s:               " << (unsigned int)(bytes / 1024.0 / 1024.0 / 1024.0 / diff_time.count()) << std::endl;
        out << "Messages/s:            " << (unsigned int)(msgs  /                            diff_time.count()) << std::endl;
        out << "Latency (us):          " << (diff_time.count() * 1e6) / (double)msgs                             << std::endl;
        std::cout << out.str() << std::endl;
        eCAL::Logging::Log(out.str());

        // reset counter
        msgs  = 0;
        bytes = 0;

        // reset start time
        start_time = std::chrono::steady_clock::now();
      }
    }
  }

  // destroy publisher
  pub.Destroy();

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
