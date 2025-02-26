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

#include <gtest/gtest.h>
#include <ecal/msg/capnproto/serializer.h>
#include <addressbook.capnp.h>

struct DataTypeInformation
{
  std::string name;          //!< name of the datatype
  std::string encoding;      //!< encoding of the datatype (e.g. protobuf, flatbuffers, capnproto)
  std::string descriptor;    //!< descriptor information of the datatype (necessary for reflection)

  //!< @cond
  bool operator==(const DataTypeInformation& other) const
  {
    return name == other.name && encoding == other.encoding && descriptor == other.descriptor;
  }

  bool operator!=(const DataTypeInformation& other) const
  {
    return !(*this == other);
  }

  bool operator<(const DataTypeInformation& rhs) const
  {
    return std::tie(name, encoding, descriptor) < std::tie(rhs.name, rhs.encoding, rhs.descriptor);
  }

  //!< @endcond
};


void buildAddressBook(AddressBook::Builder& addressBook)
{
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
  bob.setName("Bob");
  bob.setEmail("bob@example.com");

  auto bobPhones = bob.initPhones(2);
  bobPhones[0].setNumber("555-4567");
  bobPhones[0].setType(Person::PhoneNumber::Type::HOME);
  bobPhones[1].setNumber("555-7654");
  bobPhones[1].setType(Person::PhoneNumber::Type::WORK);
  bob.getEmployment().setUnemployed();
  bob.setWeight(80.8f);
}

TEST(SerializerTest, SerializeDeserializeRoundTrip)
{
  capnp::MallocMessageBuilder message;
  AddressBook::Builder addressBook = message.initRoot<AddressBook>();
  buildAddressBook(addressBook);

  eCAL::capnproto::internal::Serializer<AddressBook, DataTypeInformation> serializer;

  size_t size = serializer.MessageSize(message);
  std::vector<char> buffer(size);
  bool serializeResult = serializer.Serialize(message, buffer.data(), buffer.size());
  EXPECT_TRUE(serializeResult);

  DataTypeInformation info = serializer.GetDataTypeInformation();

  // Now deserialize.
  eCAL::capnproto::internal::Serializer<AddressBook, DataTypeInformation> deserializer;
  auto reader = deserializer.Deserialize(buffer.data(), buffer.size(), info);

  // Assert that deserialized message is the same as the original message.
  //EXPECT_EQ(reader, addressBook.asReader());
}

TEST(SerializerTest, SerializeDeserializeDynamic)
{
  capnp::MallocMessageBuilder message;
  AddressBook::Builder addressBook = message.initRoot<AddressBook>();
  buildAddressBook(addressBook);

  eCAL::capnproto::internal::Serializer<AddressBook, DataTypeInformation> serializer;

  size_t size = serializer.MessageSize(message);
  std::vector<char> buffer(size);
  bool serializeResult = serializer.Serialize(message, buffer.data(), buffer.size());
  EXPECT_TRUE(serializeResult);

  DataTypeInformation info = serializer.GetDataTypeInformation();

  eCAL::capnproto::internal::DynamicSerializer<DataTypeInformation> dynamic_deserializer;
  capnp::DynamicStruct::Reader reader = dynamic_deserializer.Deserialize(buffer.data(), buffer.size(), info);
}
