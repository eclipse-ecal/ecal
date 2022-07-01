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
#include <ecal/msg/protobuf/subscriber.h>

#include <iostream>

#include "person.pb.h"
#include <memory>

long long global_clock;

void OnPerson(const char* topic_name_, const pb::People::Person& person_, const long long time_, const long long clock_)
{
  global_clock += clock_;
  eCAL::Process::SleepMS(1);
}

int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "person subscriber");

  // set process state
  eCAL::Process::SetState(proc_sev_healthy, proc_sev_level1, "I feel good !");

  // create a subscriber (topic name "person")
  std::shared_ptr< eCAL::protobuf::CSubscriber<pb::People::Person>> sub;


  // add receive callback function (_1 = topic_name, _2 = msg, _3 = time, _4 = clock, _5 = id)
  auto callback = std::bind(OnPerson, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

  int i = 0;
  while(eCAL::Ok())
  {
    sub = std::make_shared<eCAL::protobuf::CSubscriber<pb::People::Person>>("person");
    sub->AddReceiveCallback(callback);
    // sleep 100 ms
    std::cout << "Created new publisher no " << i << std::endl;
    eCAL::Process::SleepMS(2000);
  }

  // finalize eCAL API
  eCAL::Finalize();
  
  return(0);
}
