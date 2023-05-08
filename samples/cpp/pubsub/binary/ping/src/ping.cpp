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

const int pings             = 10000;
long long diff_array[pings] = { 0 };

int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "ping");

  eCAL::CPublisher  pub_pulse("pulse_send",  "long long");
  eCAL::CSubscriber sub_pulse("pulse_reply", "long long");

  std::cout << "-------------------------------" << std::endl;
  std::cout << " PING PONG TEST                " << std::endl;
  std::cout << "-------------------------------" << std::endl;
  std::cout << "Press Enter to start .."         << std::endl;
  (void)getchar();

  std::string rec_buf;
  long long start_time_all = eCAL::Time::GetMicroSeconds();
  for(int reply = 0; reply < pings; reply++)
  {
    long long start_time_loop = eCAL::Time::GetMicroSeconds();
    pub_pulse.Send(&start_time_loop, sizeof(long long));
    if(sub_pulse.ReceiveBuffer(rec_buf, nullptr, 100))
    {
     diff_array[reply] = eCAL::Time::GetMicroSeconds() - start_time_loop;
    }
  }
  long long end_time_all   = eCAL::Time::GetMicroSeconds();
  long long diff_time_loop = 0;
  for(int reply = 0; reply < pings; reply++)
  {
    std::cout << "Pulse sent/received in " << diff_array[reply] << " us" << std::endl;
    diff_time_loop += diff_array[reply];
  }
  long long diff_time_all = end_time_all - start_time_all;
  diff_time_loop /= pings;
  diff_time_all  /= pings;
  std::cout << std::endl << "Pulse average send/receive time " << diff_time_loop << " us / " << diff_time_all << " us" << std::endl;
  std::cout << std::endl;

  // finalize eCAL API
  eCAL::Finalize();

  std::cout << "Press Enter to close .." << std::endl;
  (void)getchar();

  return(0);
}
