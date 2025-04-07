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
#include <ecal/msg/flatbuffers/publisher.h>

#include <iostream>
#include <chrono>
#include <thread>

// flatbuffers includes
#include <flatbuffers/flatbuffers.h>

// flatbuffers generated includes
#include <monster/monster_generated.h>


int main(int /*argc*/, char **/*argv*/)
{
  // initialize eCAL API
  eCAL::Initialize("monster_send");

  // set process state
  eCAL::Process::SetState(eCAL::Process::eSeverity::healthy, eCAL::Process::eSeverityLevel::level1, "I feel good !");

  // create a publisher (topic name "monster")
  eCAL::flatbuffers::CObjectPublisher<Game::Sample::MonsterT> pub("monster");
  
  Game::Sample::MonsterT my_monster;
  my_monster.name = "Monster";
  my_monster.pos = std::make_unique<Game::Sample::Vec3>( 1.0f, 2.0f, 3.0f );

  // enter main loop
  uint8_t cnt = 0;
  while(eCAL::Ok())
  {
    if (cnt == 0)
    {
      my_monster.inventory.clear();
    }
    my_monster.inventory.push_back(cnt);
    ++cnt;

    // send the monster object
    pub.Send(my_monster);

    // print content
    std::cout << "monster pos x     : " << my_monster.pos->x()         << std::endl;
    std::cout << "monster pos y     : " << my_monster.pos->y()         << std::endl;
    std::cout << "monster pos z     : " << my_monster.pos->z()         << std::endl;
    std::cout << "monster mana      : " << my_monster.mana             << std::endl;
    std::cout << "monster hp        : " << my_monster.hp               << std::endl;
    std::cout << "monster name      : " << my_monster.name             << std::endl;

    std::cout << "monster inventory : ";
    for(auto inventory : my_monster.inventory)
    {
      std::cout << (int)inventory << " ";
    }
    std::cout << std::endl;

    std::cout << "monster color     : ";
    switch (my_monster.color)
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

    // sleep 500 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  // finalize eCAL API
  eCAL::Finalize();
}
