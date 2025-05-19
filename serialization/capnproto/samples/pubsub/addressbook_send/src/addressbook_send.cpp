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
#include <ecal/msg/capnproto/publisher.h>
#include <ecal/msg/capnproto/helper.h>

#include <iostream>
#include <iterator>
#include <chrono>
#include <thread>

// capnp includes
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif /*_MSC_VER*/
#include "addressbook.capnp.h" 
#ifdef _MSC_VER
#pragma warning(pop)
#endif

void printAddressBook(AddressBook::Builder addressBook)
{
  auto m_string = addressBook.toString().flatten();
  std::cout << "our string: " << m_string.cStr() << std::endl;
}

int main()
{
  // initialize eCAL API
  eCAL::Initialize("addressbook_send");

  // set process state
  eCAL::Process::SetState(eCAL::Process::eSeverity::healthy, eCAL::Process::eSeverityLevel::level1, "I feel good !");

  // create a publisher (topic name "addressbook")
  eCAL::capnproto::CPublisher<AddressBook> pub("addressbook");

  capnp::MallocMessageBuilder message;
  AddressBook::Builder addressBook = message.initRoot<AddressBook>();
  auto people = addressBook.initPeople(2);

  auto alice = people[0];
  alice.setId(123);
  alice.setName("Alice");
  alice.setEmail("alice@example.com");

  auto alicePhones = alice.initPhones(1);
  alicePhones[0].setNumber("555-1212");
  alicePhones[0].setType(Person::PhoneNumber::Type::MOBILE);
  alice.getEmployment().setSchool("MIT");
  alice.setWeight(60.4f);
  
  kj::byte* data = new kj::byte[6];
  data[0] = 0x1F;
  data[1] = 0x00;
  data[2] = 0xA1;
  data[3] = 0xB4;
  data[4] = 0x14;
  data[5] = 0x54;
  capnp::Data::Builder aliceData(data, 6);
  alice.setData(aliceData);

  auto bob = people[1];
  int bobid = 456;
  bob.setName("Bob");
  bob.setEmail("bob@example.com");

  auto bobPhones = bob.initPhones(2);
  bobPhones[0].setNumber("555-4567");
  bobPhones[0].setType(Person::PhoneNumber::Type::HOME);
  bobPhones[1].setNumber("555-7654");
  bobPhones[1].setType(Person::PhoneNumber::Type::WORK);
  bob.getEmployment().setUnemployed();
  bob.setWeight(80.8f);

  // enter main loop
  while (eCAL::Ok())
  {
    bob.setId(++bobid);
    // send content
    pub.Send(message);

    // print content
    printAddressBook(addressBook);
    std::cout << std::endl;

    // sleep 500 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  // finalize eCAL API
  eCAL::Finalize();
}
