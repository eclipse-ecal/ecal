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
#include <ecal/msg/protobuf/subscriber.h>

#include <iostream>

#include "person.pb.h"

void OnEvent(const char* topic_name_, const struct eCAL::v5::SSubEventCallbackData* data_)
{
  std::cout << "topic name       : " << topic_name_ << std::endl;
  switch (data_->type)
  {
  case eCAL::eSubscriberEvent::connected:
    std::cout << "event            : " << "connected" << std::endl;
    break;
  case eCAL::eSubscriberEvent::disconnected:
    std::cout << "event            : " << "disconnected" << std::endl;
    break;
  case eCAL::eSubscriberEvent::dropped:
    std::cout << "event            : " << "dropped (" << data_->clock << " messages)" << std::endl;
    break;
  // not implemented yet
  case eCAL::eSubscriberEvent::corrupted:
    std::cout << "event            : " << "corrupted" << std::endl;
    break;
  case eCAL::eSubscriberEvent::update_connection:
    std::cout << "event            : " << "update_connection" << std::endl;
    std::cout << "  topic_id       : " << data_->tid << std::endl;
    std::cout << "  topic_encoding : " << data_->tdatatype.encoding << std::endl;
    std::cout << "  topic_type     : " << data_->tdatatype.name << std::endl;
    //std::cout << "  topic_desc : " << data_->tdesc << std::endl;
    break;
  default:
    std::cout << "event            : " << "unknown" << std::endl;
    break;
  }
  std::cout << std::endl;
}

int main()
{
  // initialize eCAL API
  eCAL::Initialize("person subscriber events");

  // set process state
  eCAL::Process::SetState(eCAL::Process::eSeverity::healthy, eCAL::Process::eSeverityLevel::level1, "I feel good !");

  // create a subscriber (topic name "person")
  eCAL::protobuf::CSubscriber<pb::People::Person> sub("person");

  // add event callback function (_1 = topic_name, _2 = event data struct)
  auto evt_callback = std::bind(OnEvent, std::placeholders::_1, std::placeholders::_2);
  sub.AddEventCallback(eCAL::eSubscriberEvent::connected,         evt_callback);
  sub.AddEventCallback(eCAL::eSubscriberEvent::disconnected,      evt_callback);
  sub.AddEventCallback(eCAL::eSubscriberEvent::dropped,           evt_callback);
  sub.AddEventCallback(eCAL::eSubscriberEvent::corrupted,         evt_callback);
  sub.AddEventCallback(eCAL::eSubscriberEvent::update_connection, evt_callback);

  // start application and wait for events
  std::cout << "Please start 'person_snd_events sample." << std::endl << std::endl;

  // run for 10 seconds
  auto cnt(0);
  while (eCAL::Ok() && (++cnt <= 10))
  {
    // sleep 100 ms
    eCAL::Process::SleepMS(1000);
  }

  // finalize eCAL API
  eCAL::Finalize();

  // keep alive for 10 seconds
  eCAL::Process::SleepMS(10000);

  return(0);
}
