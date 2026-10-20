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
#include <ecal/msg/protobuf/dynamic_json_subscriber.h>

#include <iostream>
#include <string>

const std::string MESSAGE_NAME("person");

void ProtoMsgCallback(const eCAL::STopicId& topic_id_, const std::string& msg_)
{
  std::cout << topic_id_.topic_name << " : " << msg_ << std::endl;
  std::cout << std::endl;
}

int main()
{
  // initialize eCAL API
  eCAL::Initialize("proto_dyn");

  // create dynamic subscribers for receiving and decoding messages
  eCAL::protobuf::CDynamicJSONSubscriber sub(MESSAGE_NAME);
  sub.SetReceiveCallback(std::bind(ProtoMsgCallback, std::placeholders::_1, std::placeholders::_2));

  // enter main loop
  while(eCAL::Ok())
  {
    // sleep main thread for 1 second
    eCAL::Process::SleepMS(1000);
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
