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

/**
 * Tests eCALHDF5 reader w/o input file (single channel)
**/

#include <ecal/protobuf/ecal_proto_decoder.h>
#include <ecal/protobuf/ecal_proto_maximum_array_dimensions.h>

#include <gtest/gtest.h>

#include <cstdio>
#include <memory>

#include "person.pb.h"

using namespace eCAL::protobuf;

TEST(MaximumArrayDimensions, Standard)
{
  pb::People::Person person;

  // set person object content
  person.set_id(42);
  person.set_name("Max");
  person.set_stype(pb::People::Person_SType_MALE);
  person.set_email("max@mail.net");
  person.mutable_dog()->set_name("Brandy");
  person.mutable_house()->set_rooms(4);

  // We want to test, if the 
  CProtoDecoder decoder;
  auto visitor = std::make_shared<MaximumArrayDimensionsVisitor>();
  decoder.SetVisitor(visitor);

  // Add a different number of favorite numbers and then Test what the maximum was.
  ASSERT_EQ(0, visitor->MaxSize("favorite_numbers"));

  person.mutable_favorite_numbers()->Resize(1, 0);
  decoder.ProcProtoMsg(person);
  ASSERT_EQ(1, visitor->MaxSize("favorite_numbers"));

  person.mutable_favorite_numbers()->Resize(7, 0);
  decoder.ProcProtoMsg(person);
  ASSERT_EQ(7, visitor->MaxSize("favorite_numbers"));

  person.mutable_favorite_numbers()->Resize(5, 0);
  decoder.ProcProtoMsg(person);
  ASSERT_EQ(7, visitor->MaxSize("favorite_numbers"));
}

