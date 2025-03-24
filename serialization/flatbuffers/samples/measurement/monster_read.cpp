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

#include <ecal/msg/flatbuffers/imeasurement.h>

#include <iostream>

#include "monster/monster_generated.h"

void PrintMonster(const Game::Sample::Monster* monster_)
{
  // print content
  std::cout << "monster pos x     : " << monster_->pos()->x() << std::endl;
  std::cout << "monster pos y     : " << monster_->pos()->y() << std::endl;
  std::cout << "monster pos z     : " << monster_->pos()->z() << std::endl;
  std::cout << "monster mana      : " << monster_->mana() << std::endl;
  std::cout << "monster hp        : " << monster_->hp() << std::endl;
  std::cout << "monster name      : " << monster_->name()->c_str() << std::endl;

  std::cout << "monster inventory : ";
  for (auto iter = monster_->inventory()->begin(); iter != monster_->inventory()->end(); ++iter)
  {
    std::cout << static_cast<int>(*iter) << " ";
  }
  std::cout << std::endl;

  std::cout << "monster color     : ";
  switch (monster_->color())
  {
  case Game::Sample::Color_Red:
    std::cout << "Red";
    break;
  case Game::Sample::Color_Green:
    std::cout << "Green";
    break;
  case Game::Sample::Color_Blue:
    std::cout << "Blue";
    break;
  }
  std::cout << std::endl;
  std::cout << std::endl;
}

void PrintMonster(const Game::Sample::MonsterT* monster_)
{
  // print content
  std::cout << "monster pos x     : " << monster_->pos->x() << std::endl;
  std::cout << "monster pos y     : " << monster_->pos->y() << std::endl;
  std::cout << "monster pos z     : " << monster_->pos->z() << std::endl;
  std::cout << "monster mana      : " << monster_->mana << std::endl;
  std::cout << "monster hp        : " << monster_->hp << std::endl;
  std::cout << "monster name      : " << monster_->name << std::endl;

  std::cout << "monster inventory : ";
  for (const auto& inventory : monster_->inventory)
  {
    std::cout << (int)inventory << " ";
  }
  std::cout << std::endl;

  std::cout << "monster color     : ";
  switch (monster_->color)
  {
  case Game::Sample::Color_Red:
    std::cout << "Red";
    break;
  case Game::Sample::Color_Green:
    std::cout << "Green";
    break;
  case Game::Sample::Color_Blue:
    std::cout << "Blue";
    break;
  }
  std::cout << std::endl;
  std::cout << std::endl;
}

int main(int /*argc*/, char** /*argv*/)
{
  // create a new measurement
  eCAL::measurement::IMeasurement meas(".");

  // create a channel (topic name "person")
  auto monster_channels = meas.Channels("monster");
  if (monster_channels.size() > 0)
  {
    // There are two ways that we can extract data from the measurement

    {
      // The first way is via the flat API, which directly maps the fields to C++ Objects
      eCAL::flatbuffers::IChannel<Game::Sample::Monster> monster_channel{ eCAL::measurement::GetChannel<eCAL::flatbuffers::IChannel<Game::Sample::Monster>>(meas, *monster_channels.begin()) };
      // iterate over the messages
      for (const auto& monster_entry : monster_channel)
      {
        std::cout << "Monster object at timestamp " << monster_entry.send_timestamp << std::endl;
        PrintMonster(monster_entry.message);
      }
    }

    {
      // The second one is via the object API, which directly maps the fields to C++ Objects
      // Generated object types are postfixed with "T"
      eCAL::flatbuffers::IChannel<Game::Sample::MonsterT> monster_channel{ eCAL::measurement::GetChannel<eCAL::flatbuffers::IChannel<Game::Sample::MonsterT>>(meas, *monster_channels.begin()) };
      // iterate over the messages
      for (const auto& monster_entry : monster_channel)
      {
        std::cout << "Monster object at timestamp " << monster_entry.send_timestamp << std::endl;
        PrintMonster(monster_entry.message);
      }
    }
  }
  return 0;
}
