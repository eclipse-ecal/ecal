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
#include <ecal/msg/messagepack/publisher.h>

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

int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "address publisher");

  // set process state
  eCAL::Process::SetState(proc_sev_healthy, proc_sev_level1, "I feel good !");

  // create a publisher (topic name "address")
  eCAL::messagepack::CPublisher<CAddress> pub("address");

  // create the message
  CAddress address;
  address.fname         = "Peter";
  address.lname         = "Miller";
  address.street        = "Continental drive";
  address.street_number = 1;
  address.town          = "Auburn Hills";
  address.country       = "USA";

  // enter main loop
  while (eCAL::Ok())
  {
    // send content
    pub.Send(address, -1);

    // print content
    std::cout << "first name     : " << address.fname         << std::endl;
    std::cout << "last name      : " << address.lname         << std::endl;
    std::cout << "street         : " << address.street        << std::endl;
    std::cout << "street_number  : " << address.street_number << std::endl;
    std::cout << "town           : " << address.town          << std::endl;
    std::cout << "country        : " << address.country       << std::endl;
    std::cout                                                 << std::endl;

    // sleep 500 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  // finalize eCAL API
  eCAL::Finalize();
}
