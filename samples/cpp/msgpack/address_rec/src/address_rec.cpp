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
#include <ecal/msg/messagepack/subscriber.h>

#include <iostream>
#include <chrono>
#include <thread>

class CAddress
{
public:
  CAddress() :
    street_number(0)
  {};

  std::string fname;
  std::string lname;
  std::string street;
  int         street_number;
  std::string town;
  std::string country;

  MSGPACK_DEFINE(fname, lname, street, street_number, town, country);
};

void OnAddress(const char* topic_name_, const CAddress& msg_, const long long time_)
{
  std::cout << "topic name     : " << topic_name_        << std::endl;
  std::cout << "time           : " << time_              << std::endl;
  std::cout << "first name     : " << msg_.fname         << std::endl;
  std::cout << "last name      : " << msg_.lname         << std::endl;
  std::cout << "street         : " << msg_.street        << std::endl;
  std::cout << "street_number  : " << msg_.street_number << std::endl;
  std::cout << "town           : " << msg_.town          << std::endl;
  std::cout << "country        : " << msg_.country       << std::endl;
  std::cout                                              << std::endl;
}
  
int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "address subscriber");

  // set process state
  eCAL::Process::SetState(proc_sev_healthy, proc_sev_level1, "I feel good !");

  // create a subscriber (topic name "address")
  eCAL::messagepack::CSubscriber<CAddress> sub("address");

  // add receive callback function (_1 = topic_name, _2 = msg, _3 = time)
  auto callback = std::bind(OnAddress, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  sub.AddReceiveCallback(callback);

  // enter main loop
  while (eCAL::Ok())
  {
    // sleep 500 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  // finalize eCAL API
  eCAL::Finalize();
}
