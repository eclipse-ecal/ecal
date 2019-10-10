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

// globals
long long  g_clock(0);
long long  g_first_clock(-1);

// subscriber callback function
void OnReceive(const char* /* topic_name_ */, const struct eCAL::SReceiveCallbackData* data_)
{
  long long clock = ((long long*)data_->buf)[0];
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
}

int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "counter_rec_cb");

  // create subscriber for topic "Counter"
  eCAL::CSubscriber sub("Counter", "long long");

  // setup receive callback function
  sub.AddReceiveCallback(OnReceive);

  // idle main thread
  while(eCAL::Ok())
  {
    // sleep 100 ms
    eCAL::Process::SleepMS(100);
  }

  // destroy subscriber
  sub.Destroy();

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
