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

#include <ecal/ecal.h>

#include <iostream>

int main()
{
  // initialize eCAL API
  eCAL::Initialize("counter_rec_cb");

  // create subscriber for topic "Counter"
  eCAL::SDataTypeInformation datatype_information;
  datatype_information.encoding = "long long";
  eCAL::CSubscriber sub("Counter", datatype_information);

  // counter
  long long  g_clock(0);
  long long  g_first_clock(-1);

  // add callback
  auto on_receive = [&](const struct eCAL::SReceiveCallbackData& data_) {
    long long const clock = static_cast<const long long*>(data_.buffer)[0];
    if(g_first_clock < 0)
    {
      g_first_clock = clock;
    }

    if(g_clock != clock - g_first_clock)
    {
      std::cout << "Out of sync : " << (clock - g_first_clock) - g_clock << std::endl;
      g_first_clock = -1;
      g_clock = 0;
    }
    else
    {
      g_clock++;
    }
  };
  sub.SetReceiveCallback(std::bind(on_receive, std::placeholders::_3));

  // idle main thread
  while(eCAL::Ok())
  {
    // sleep 100 ms
    eCAL::Process::SleepMS(100);
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
