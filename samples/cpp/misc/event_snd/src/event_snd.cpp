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

int main(int /*argc*/, char** /*argv*/)
{
  // global parameter
  const std::string event_name = "my_event";

  // create named event
  eCAL::EventHandleT event_handle;
  eCAL::gOpenEvent(&event_handle, event_name);

  // timer
  auto start_time(std::chrono::steady_clock::now());
  int  clock(0);
  int  msgs(0);

  // send updates
  for(;;)
  {
    // fire event
    gSetEvent(event_handle);

    // collect data
    clock++;
    msgs++;

    // check timer and print results every second
    if(clock%10011 == 0)
    {
      std::chrono::duration<double> diff_time = std::chrono::steady_clock::now() - start_time;
      if(diff_time >= std::chrono::seconds(1))
      {
        start_time = std::chrono::steady_clock::now();
        std::stringstream out;
        out << "Events/s: " << int(msgs / diff_time.count()) << std::endl;
        std::cout << out.str() << std::endl;
        msgs  = 0;
      }
    }
  }
}
