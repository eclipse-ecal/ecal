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

int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "pong");

  eCAL::CSubscriber sub_pulse("pulse_send",  "long long");
  eCAL::CPublisher  pub_pulse("pulse_reply", "long long");

  std::cout << "-------------------------------" << std::endl;
  std::cout << " PING PONG TEST                " << std::endl;
  std::cout << "-------------------------------" << std::endl;
  const int pings = 10000;
  long long diff_array[pings] = {0};
  std::string rec_buf;
  for(int reply = 0; reply < pings; reply++)
  {
    if(sub_pulse.ReceiveBuffer(rec_buf, nullptr, -1))
    {
      diff_array[reply] = eCAL::Time::GetMicroSeconds() - *((long long*)rec_buf.c_str());
      pub_pulse.Send(&diff_array[reply], sizeof(long long));
    }
  }
  long long diff_time = 0;
  for(int reply = 0; reply < pings; reply++)
  {
    std::cout << "Pulse received in " << diff_array[reply] << " us" << std::endl;
    diff_time += diff_array[reply];
  }
  diff_time /= pings;
  std::cout << std::endl << "Pulse average receive time " << diff_time << " us" << std::endl;
  std::cout << std::endl;

  // finalize eCAL API
  eCAL::Finalize();

  std::cout << "Press Enter to close .." << std::endl;
  getchar();

  return(0);
}
