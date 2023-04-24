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

// capnp includes
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif /*_MSC_VER*/
#include "addressbook.capnp.h" 
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <ecal/ecal.h>
#include <ecal/msg/capnproto/subscriber.h>

#include <iostream>
#include <chrono>
#include <thread>



void printAddressBook(const AddressBook::Reader& addressBook)
{
  for (Person::Reader person : addressBook.getPeople())
  {
    std::cout << person.getName().cStr() << ": " << person.getEmail().cStr() << std::endl;
    for (Person::PhoneNumber::Reader phone : person.getPhones())
    {
      const char* typeName = "UNKNOWN";
      switch (phone.getType()) {
      case Person::PhoneNumber::Type::MOBILE: typeName = "mobile"; break;
      case Person::PhoneNumber::Type::HOME:   typeName = "home";   break;
      case Person::PhoneNumber::Type::WORK:   typeName = "work";   break;
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

    std::cout << std::endl;
  }
}

int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "addressbook subscriber");

  // set process state
  eCAL::Process::SetState(proc_sev_healthy, proc_sev_level1, "I feel good !");

  // create a subscriber (topic name "addressbook")
  eCAL::capnproto::CSubscriber<AddressBook> sub("addressbook");
  // enter main loop
  while (eCAL::Ok())
  {
    // receive content
    if (sub.Receive(nullptr, 0))
    {
      AddressBook::Reader reader{ sub.getReader() };
      printAddressBook(reader);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // finalize eCAL API
  eCAL::Finalize();
}
