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

#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <vector>

// performance settings
bool   zero_copy              (true);
int    buffer_count           (1);
int    acknowledge_timeout_ms (50);
size_t payload_size           (8* 1024 * 1024);

// hello world payload
class CHelloWorldPayload : public eCAL::payload
{
public:
  CHelloWorldPayload(size_t size_)
  {
    payload.resize(size_);
    std::fill(payload.begin(), payload.end(), (char)(42));
    std::cout << "Message size  =  " << int(payload.size()) << " Byte = " << int(payload.size() / 1024) << " kByte = " << int(payload.size() / 1024 / 1024) << " MByte" << std::endl;
  }

  void write_complete(void* buf_, size_t len_)
  {
    // write complete content
    if (len_ >= payload.size())
    {
      memcpy(buf_, payload.data(), payload.size());
    }
  };

  void write_partial(void* buf_, size_t len_)
  {
    // write partial content
    if (len_ > 0)
    {
      static_cast<char*>(buf_)[((clock++)%1024)%len_] = clock%10+48;
    }
  };

  const void* data() { return payload.data(); };
  size_t      size() { return payload.size(); };

private:
  std::vector<char> payload;
  int               clock = 0;
};

// main entry
int main(int argc, char **argv)
{
  if(argc > 1) payload_size = atoi(argv[1]);
  if(payload_size < 1) payload_size = 1;

  // initialize eCAL API
  eCAL::Initialize(argc, argv, "performance_snd");

  // create payload
  CHelloWorldPayload send_p(payload_size);

  // create publisher
  eCAL::CPublisher pub("Performance");
 
  // enable zero copy mode
  if (zero_copy)
  {
    std::cout << "Switch zero copy mode on" << std::endl;
  }
  pub.ShmEnableZeroCopy(zero_copy);

  // set write buffer count
  if (buffer_count > 1)
  {
    std::cout << "Set number of write buffer to " << buffer_count << std::endl;
  }
  pub.ShmSetBufferCount(buffer_count);
  
  // enable handshake mode
  if (acknowledge_timeout_ms > 0)
  {
    std::cout << "Activate acknowledge with " << acknowledge_timeout_ms << " ms" << std::endl;
  }
  pub.ShmSetAcknowledgeTimeout(acknowledge_timeout_ms);
  std::cout << std::endl;

  // counter
  int                clock(0);
  int                msgs (0);
  unsigned long long bytes(0);

  // set start time
  auto start_time = std::chrono::steady_clock::now();

  // send updates
  while(eCAL::Ok())
  {
    // send content
    size_t snd_len(0);
    snd_len = pub.Send(send_p);

    if((snd_len > 0) && (snd_len != send_p.size()))
    {
      std::cerr <<  std::endl << "Send failed !" << " sent : " << send_p.size() << " returned : " << snd_len <<  std::endl;
    }

    // manage counters
    clock++;
    msgs++;
    bytes += send_p.size();

    // check timer and print results every second
    if(clock%2000 == 0)
    {
      std::chrono::duration<double> diff_time = std::chrono::steady_clock::now() - start_time;
      if(diff_time >= std::chrono::seconds(1))
      {
        // log results
        std::stringstream out;
        out << "Message size (kByte):  " << (unsigned int)(send_p.size() / 1024)                           << std::endl;
        out << "kByte/s:               " << (unsigned int)(bytes / 1024 /               diff_time.count()) << std::endl;
        out << "MByte/s:               " << (unsigned int)(bytes / 1024 / 1024 /        diff_time.count()) << std::endl;
        out << "GByte/s:               " << (unsigned int)(bytes / 1024 / 1024 / 1024 / diff_time.count()) << std::endl;
        out << "Messages/s:            " << (unsigned int)(msgs  /                      diff_time.count()) << std::endl;
        out << "Latency (us):          " << (diff_time.count() / msgs) * 1000 * 1000                       << std::endl;
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
