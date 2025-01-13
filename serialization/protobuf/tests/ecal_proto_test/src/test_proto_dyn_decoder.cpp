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

#include <ecal/protobuf/ecal_proto_decoder.h>
#include <ecal/protobuf/ecal_proto_message_filter.h>
#include <ecal/protobuf/ecal_proto_visitor.h>

#include <gtest/gtest.h>

#include <cstdio>

#include "person.pb.h"

using namespace eCAL::protobuf;

std::vector<std::string> elems;
std::vector<size_t> values;

void clear_output()
{
  elems.clear();
  values.clear();
}

class TestVisitor : public eCAL::protobuf::MessageVisitorDoubleIntegral
{
public:
  TestVisitor() :filter(std::make_shared<eCAL::protobuf::NoFilter>()) {};
  std::shared_ptr<eCAL::protobuf::MessageFilter> filter;

protected:
  using eCAL::protobuf::MessageVisitorDoubleIntegral::ScalarValueIntegral;
  virtual void ScalarValueIntegral(const std::string& field_, const std::string& /*group_*/, double /*value_*/) override
  {
    elems.push_back(field_);
  };
  virtual void ScalarValueEnum(const std::string& field_, const std::string& group_, int value_) override
  {
    ScalarValueIntegral(field_, group_, static_cast<double>(value_));
  };
  virtual void ArrayValueIntegral(size_t index_, double /*value_*/) override
  {
    values.push_back(index_);
  };

  virtual bool AcceptMessage(const std::string& message) override
  {
    return filter->Filter(message);
  };
};

void add_favorite_numbers(pb::People::Person& person)
{
  person.mutable_favorite_numbers()->Resize(5, 0);
  for (int num = 0; num < 5; num++)
  {
    person.mutable_favorite_numbers()->Set(num, num);
  }
}


TEST(protodecoder, no_filter)
{
  // generate a class instance of Person
  pb::People::Person person;

  // set person object content
  person.set_id(42);
  person.set_name("Max");
  person.set_stype(pb::People::Person_SType_MALE);
  person.set_email("max@mail.net");
  person.mutable_dog()->set_name("Brandy");
  person.mutable_house()->set_rooms(4);

  CProtoDecoder decoder;
  auto visitor = std::make_shared<TestVisitor>();
  decoder.SetVisitor(visitor);

  clear_output();
  decoder.ProcProtoMsg(person);
  std::vector<std::string> expected_all{ "id", "stype", "rooms" };
  EXPECT_EQ(expected_all, elems);

  clear_output();
  auto filter = std::make_shared<ComplexIncludeFilter>();
  filter->Insert("id");
  visitor->filter= filter;

  decoder.ProcProtoMsg(person);
  std::vector<std::string> expected_id{ "id" };
  EXPECT_EQ(expected_id, elems);

  clear_output();
  filter->Insert("house.rooms");
  decoder.ProcProtoMsg(person);
  std::vector<std::string> expected_id_rooms{ "id", "rooms" };
  EXPECT_EQ(expected_id_rooms, elems);

  // Now lets add some array members, and see how it performs with the filter.
  add_favorite_numbers(person);

  // Test case, assert the array callback was called only once
  clear_output();
  filter->Clear();
  filter->Insert("favorite_numbers[1]");
  decoder.ProcProtoMsg(person);
  std::vector<size_t> expected_values_numbers{ 1 };
  EXPECT_EQ(expected_values_numbers, values);
  
  // Test case, assert the array callback was called for each element
  clear_output();
  filter->Clear();
  filter->Insert("favorite_numbers[*]");
  decoder.ProcProtoMsg(person);
  std::vector<size_t> expected_values_all_numbers{ 0, 1, 2, 3, 4 };
  EXPECT_EQ(expected_values_all_numbers, values);

}