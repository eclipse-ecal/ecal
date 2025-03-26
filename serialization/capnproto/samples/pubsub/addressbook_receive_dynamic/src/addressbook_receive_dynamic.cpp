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

#include <iostream>
#include <chrono>
#include <thread>

#include <ecal/ecal.h>
#include <ecal/msg/capnproto/dynamic.h>


void dynamicPrintValue(const capnp::DynamicValue::Reader& value)
{
  using namespace capnp;
  // Print an arbitrary message via the dynamic API by
  // iterating over the schema.  Look at the handling
  // of STRUCT in particular.

  switch (value.getType()) {
  case DynamicValue::VOID:
    std::cout << "";
    break;
  case DynamicValue::BOOL:
    std::cout << (value.as<bool>() ? "true" : "false");
    break;
  case DynamicValue::INT:
    std::cout << value.as<int64_t>();
    break;
  case DynamicValue::UINT:
    std::cout << value.as<uint64_t>();
    break;
  case DynamicValue::FLOAT:
    std::cout << value.as<double>();
    break;
  case DynamicValue::TEXT:
    std::cout << '\"' << value.as<Text>().cStr() << '\"';
    break;
  case DynamicValue::LIST: {
    std::cout << "[";
    bool first = true;
    for (auto element : value.as<DynamicList>()) {
      if (first) {
        first = false;
      }
      else {
        std::cout << ", ";
      }
      dynamicPrintValue(element);
    }
    std::cout << "]";
    break;
  }
  case DynamicValue::ENUM: {
    auto enumValue = value.as<DynamicEnum>();
    KJ_IF_MAYBE(enumerant, enumValue.getEnumerant()) {
      std::cout <<
        enumerant->getProto().getName().cStr();
    }
 else {
   // Unknown enum value; output raw number.
   std::cout << enumValue.getRaw();
 }
 break;
  }
  case DynamicValue::STRUCT: {
    std::cout << "(";
    auto structValue = value.as<DynamicStruct>();
    bool first = true;
    for (auto field : structValue.getSchema().getFields()) {
      if (!structValue.has(field)) continue;
      if (first) {
        first = false;
      }
      else {
        std::cout << ", ";
      }
      std::cout << field.getProto().getName().cStr()
        << " = ";
      dynamicPrintValue(structValue.get(field));
    }
    std::cout << ")";
    std::cout << "\n";
    break;
  }
  default:
    // There are other types, we aren't handling them.
    std::cout << "?";
    break;
  }
}

int main()
{
  // initialize eCAL API
  eCAL::Initialize("addressbook_receive_dynamic");

  eCAL::Process::SetState(eCAL::Process::eSeverity::healthy, eCAL::Process::eSeverityLevel::level1, "I feel good !");

  // create a subscriber (topic name "addressbook")
  eCAL::capnproto::CDynamicSubscriber sub("addressbook");

  auto lambda = [](const eCAL::STopicId& /*topic_id_*/, const capnp::DynamicValue::Reader& msg_, long long /*time_*/, long long /*clock_*/) -> void {
    dynamicPrintValue(msg_);
  };
  sub.SetReceiveCallback(lambda);

  // enter main loop
  while (eCAL::Ok())
  {
    // sleep 500 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  // finalize eCAL API
  eCAL::Finalize();
  
  return(0);
}
