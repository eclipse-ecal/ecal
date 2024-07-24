/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

void OnPerson(const char* topic_name_, const pb::People::Person& person_, const long long time_, const long long clock_)
{
  std::cout << "------------------------------------------" << std::endl;
  std::cout << " HEAD "                                     << std::endl;
  std::cout << "------------------------------------------" << std::endl;
  std::cout << "topic name   : " << topic_name_             << std::endl;
  std::cout << "topic time   : " << time_                   << std::endl;
  std::cout << "topic clock  : " << clock_                  << std::endl;
  std::cout << "------------------------------------------" << std::endl;
  std::cout << " CONTENT "                                  << std::endl;
  std::cout << "------------------------------------------" << std::endl;
  std::cout << "person id    : " << person_.id()            << std::endl;
  std::cout << "person name  : " << person_.name()          << std::endl;
  std::cout << "person stype : " << person_.stype()         << std::endl;
  std::cout << "person email : " << person_.email()         << std::endl;
  std::cout << "dog.name     : " << person_.dog().name()    << std::endl;
  std::cout << "house.rooms  : " << person_.house().rooms() << std::endl;
  std::cout << "------------------------------------------" << std::endl;
  std::cout                                                 << std::endl;
}

int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "person subscriber");

  // set process state
  eCAL::Process::SetState(proc_sev_healthy, proc_sev_level1, "I feel good !");

  // create a subscriber config
  eCAL::Subscriber::Configuration sub_config;

  // activate transport layer
  sub_config.layer.shm.enable = true;
  sub_config.layer.udp.enable = true;
  sub_config.layer.tcp.enable = true;

  // create a subscriber (topic name "person")
  eCAL::protobuf::CSubscriber<pb::People::Person> sub("person", sub_config);

  // add receive callback function (_1 = topic_name, _2 = msg, _3 = time, _4 = clock, _5 = id)
  auto callback = std::bind(OnPerson, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
  sub.AddReceiveCallback(callback);

  while(eCAL::Ok())
  {
    // sleep 100 ms
    eCAL::Process::SleepMS(100);
  }

  // finalize eCAL API
  eCAL::Finalize();
  
  return(0);
}
