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

#include <ecal/msg/capnproto/imeasurement.h>

#include <iostream>
#include <capnp/pretty-print.h>

int main(int /*argc*/, char** /*argv*/)
{
  // create a new measurement
  eCAL::measurement::IMeasurement meas(".");

  // create a channel (topic name "person")
  auto addressbook_channels = meas.Channels("addressbook");
  if (addressbook_channels.size() > 0)
  {
    // create a dynamic channel
    eCAL::capnproto::dynamic::IChannel addressbook_channel{ eCAL::measurement::GetChannel<eCAL::capnproto::dynamic::IChannel>(meas, *addressbook_channels.begin()) };

    // iterate over the messages
    for (const auto& addressbook_entry : addressbook_channel)
    {
      std::cout << "Person object at timestamp " << addressbook_entry.send_timestamp << std::endl;
      auto addressbook_string = capnp::prettyPrint(addressbook_entry.message);
      std::cout << kj::str(addressbook_string).cStr() << std::endl;
    }
  }
  return 0;
}
