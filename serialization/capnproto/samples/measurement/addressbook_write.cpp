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

#include <ecal/msg/capnproto/omeasurement.h>

#include <iostream>

#include "addressbook.capnp.h"

constexpr auto ONE_SECOND = 1000000;

// Function that fills the Addressbook with data
void PrepareAddressbook(capnp::MallocMessageBuilder& message, int bob_id)
{
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
  bob.setId(bob_id);
}

int main(int /*argc*/, char** /*argv*/)
{
  // create a new measurement
  eCAL::measurement::OMeasurement meas(".");

  // create a channel (topic name "addressbook")
  eCAL::capnproto::OChannel<AddressBook> address_book_channel = eCAL::measurement::CreateChannel<eCAL::capnproto::OChannel<AddressBook>>(meas, "addressbook");

  long long timestamp = 0;

  for (int i = 0; i < 100; i++)
  {
    // In order to write a message, the user will later write the message builder to the measurement
    capnp::MallocMessageBuilder message;
    PrepareAddressbook(message, i);

    // the message can now be piped into the measurement
    address_book_channel << eCAL::measurement::make_frame(message, timestamp);
    timestamp += ONE_SECOND;
  }

  return 0;
}
