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

#include <string>

int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "dynsize_snd");

  // publisher for topic "Performance"
  eCAL::CPublisher pub("Performance");

  // prepare snd_buf
  const int MAX_BUFSIZE(10*1024*1024);
  std::vector<char> snd_buf(MAX_BUFSIZE);
  for(int i = 0; i < MAX_BUFSIZE; ++i)
  {
    switch (i % 4)
    {
    case 0:
      snd_buf[i] = 'A';
      break;
    case 1:
      snd_buf[i] = 'B';
      break;
    case 2:
      snd_buf[i] = 'C';
      break;
    case 3:
      snd_buf[i] = 'D';
      break;
    }
  }

  // send updates
  size_t max_size(MAX_BUFSIZE);
  for(int cnt = 1; cnt < MAX_BUFSIZE; cnt += 1 + cnt/10000)
  {
    // first we increase send size byte wise
    // later we keep size constant for a while
    size_t send_size(cnt);
    if(cnt > 10*1024)
    {
      send_size = int(cnt/10240) * 10240;
    }
    if(send_size > max_size)
    {
      send_size = max_size;
    }

    // send content
    size_t snd_len = pub.Send(snd_buf.data(), send_size, cnt);
    if((snd_len > 0) && (snd_len != send_size))
    {
      std::cout << std::endl << "Sending failed !" << std::endl;
    }

    // print content
    if(cnt%10000 == 0)
    {
      if(send_size < 6*1024)
      {
        std::cout << std::endl << "Sent message size: " << send_size        << " Byte"  << std::endl;
      }
      else
      {
        std::cout << std::endl << "Sent message size: " << send_size/1024.0 << " kByte" << std::endl;
      }
    }

    // sleep 1 s
    if(cnt%1000 == 0) eCAL::Process::SleepMS(1000);
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
