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
#include <ecal/msg/capnproto/publisher.h>

#include <iostream>
#include <chrono>
#include <thread>

// capnp includes
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4239)
#endif
#include "addressbook.capnp.h" 
#ifdef _MSC_VER
#pragma warning(pop)
#endif

void printAddressBook(capnp::MallocMessageBuilder& msg_)
{
  AddressBook::Reader addressBook = msg_.getRoot<AddressBook>();

  for (Person::Reader person : addressBook.getPeople())
  {
    std::cout << person.getName().cStr() << ": " << person.getEmail().cStr() << std::endl;
    for (Person::PhoneNumber::Reader phone : person.getPhones())
    {
      const char* typeName = "UNKNOWN";
      switch (phone.getType()) {
      case Person::PhoneNumber::Type::MOBILE: typeName = "mobile"; break;
      case Person::PhoneNumber::Type::HOME: typeName = "home"; break;
      case Person::PhoneNumber::Type::WORK: typeName = "work"; break;
      }
      std::cout << "  " << typeName << " phone: " << phone.getNumber().cStr() << std::endl;
    }
    Person::Employment::Reader employment = person.getEmployment();

    switch (employment.which())
    {
    case Person::Employment::UNEMPLOYED:
      std::cout << "  unemployed" << std::endl;
      break;
    case Person::Employment::EMPLOYER:
      std::cout << "  employer: "
        << employment.getEmployer().cStr() << std::endl;
      break;
    case Person::Employment::SCHOOL:
      std::cout << "  student at: "
        << employment.getSchool().cStr() << std::endl;
      break;
    case Person::Employment::SELF_EMPLOYED:
      std::cout << "  self-employed" << std::endl;
      break;
    }
  }
}

int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "addressbook publisher");

  // set process state
  eCAL::Process::SetState(proc_sev_healthy, proc_sev_level1, "I feel good !");

  // create a publisher (topic name "addressbook")
  eCAL::CCapnpPublisher<capnp::MallocMessageBuilder> pub("addressbook");

  // create a capnp message
  capnp::MallocMessageBuilder message;

  auto addressBook = message.initRoot<AddressBook>();
  auto people = addressBook.initPeople(2);

  auto alice = people[0];
  alice.setId(123);
  alice.setName("Alice");
  alice.setEmail("alice@example.com");

  auto alicePhones = alice.initPhones(1);
  alicePhones[0].setNumber("555-1212");
  alicePhones[0].setType(Person::PhoneNumber::Type::MOBILE);
  alice.getEmployment().setSchool("MIT");

  auto bob = people[1];
  bob.setId(456);
  bob.setName("Bob");
  bob.setEmail("bob@example.com");

  auto bobPhones = bob.initPhones(2);
  bobPhones[0].setNumber("555-4567");
  bobPhones[0].setType(Person::PhoneNumber::Type::HOME);
  bobPhones[1].setNumber("555-7654");
  bobPhones[1].setType(Person::PhoneNumber::Type::WORK);
  bob.getEmployment().setUnemployed();

  // enter main loop
  while (eCAL::Ok())
  {
    // send content
    pub.Send(message, -1);

    // print content
    printAddressBook(message);
    std::cout << std::endl;

    // sleep 500 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  // finalize eCAL API
  eCAL::Finalize();
}
