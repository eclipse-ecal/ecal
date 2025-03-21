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

#include <ecal/msg/flatbuffers/omeasurement.h>

#include <iostream>

#include "monster/monster_generated.h"

constexpr auto ONE_SECOND = 1000000;

int main(int /*argc*/, char** /*argv*/)
{
  // create a new measurement
  eCAL::measurement::OMeasurement meas(".");

  // create a channel (topic name "addressbook")
  eCAL::flatbuffers::OChannel<Game::Sample::MonsterT> monster_channel = eCAL::measurement::CreateChannel<eCAL::flatbuffers::OChannel<Game::Sample::MonsterT>>(meas, "monster");

  long long timestamp = 0;

  Game::Sample::MonsterT message;
  message.name = "Monster";
  message.pos = std::make_unique<Game::Sample::Vec3>(1.0f, 2.0f, 3.0f);

  for (uint8_t i = 0; i < 100; i++)
  {
    message.inventory.push_back(i);

    // the message can now be piped into the measurement
    monster_channel << eCAL::measurement::make_frame(message, timestamp);
    timestamp += ONE_SECOND;
  }

  return 0;
}
