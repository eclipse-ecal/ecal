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
#include <ecal/pubsub/publisher.h>

#include <iostream>
#include <functional>

const int timout_ms = 10;

int main()
{
  // initialize eCAL API
  eCAL::Initialize("person publisher");
  // set process state
  eCAL::Process::SetState(eCAL::Process::eSeverity::healthy, eCAL::Process::eSeverityLevel::level1, "I feel good !");

  eCAL::CPublisher pub("hello");
  eCAL::CTimer timer;

  timer.Start(timout_ms, [&pub](){
    pub.Send("Hello world");
    std::cout << "Sent: Hello world" << std::endl;
  });

  // enter main loop
  while(eCAL::Ok())
  {
    // sleep 500 ms
    eCAL::Process::SleepMS(500);
  }

  timer.Stop();
  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}

