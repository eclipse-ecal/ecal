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

#include <ecal/ecal.h>
#include <ecal/msg/flatbuffers/publisher.h>

#include <iostream>
#include <chrono>
#include <thread>

// flatbuffers includes
#include <flatbuffers/flatbuffers.h>

// flatbuffers generated includes
#include "monster_generated.h"


int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "monster publisher");

  // set process state
  eCAL::Process::SetState(proc_sev_healthy, proc_sev_level1, "I feel good !");

  // create a publisher (topic name "monster")
  eCAL::flatbuffers::CPublisher<flatbuffers::FlatBufferBuilder> pub("monster");

  // the generic builder instance
  flatbuffers::FlatBufferBuilder builder;

  // generate a class instance of Monster
  auto vec = Game::Sample::Vec3(1, 2, 3);

  auto name = builder.CreateString("Monster");

  unsigned char inv_data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  auto inventory = builder.CreateVector(inv_data, 10);

  // shortcut for creating monster with all fields set:
  auto mloc = Game::Sample::CreateMonster(builder, &vec, 150, 80, name, inventory, Game::Sample::Color_Blue);

  builder.Finish(mloc);

  // enter main loop
  auto cnt = 0;
  while(eCAL::Ok())
  {
    // mutate elements
    auto monster(Game::Sample::GetMutableMonster(builder.GetBufferPointer()));
    monster->mutate_hp(static_cast<int16_t>(++cnt));   // set table field.
    monster->mutable_pos()->mutate_z(42);              // set struct field.
    monster->mutable_inventory()->Mutate(0, 42);       // set vector element.

    // send the monster object
    pub.Send(builder, -1);

    // print content
    std::cout << "monster pos x     : " << monster->pos()->x()       << std::endl;
    std::cout << "monster pos y     : " << monster->pos()->y()       << std::endl;
    std::cout << "monster pos z     : " << monster->pos()->z()       << std::endl;
    std::cout << "monster mana      : " << monster->mana()           << std::endl;
    std::cout << "monster hp        : " << monster->hp()             << std::endl;
    std::cout << "monster name      : " << monster->name()->c_str()  << std::endl;

    std::cout << "monster inventory : ";
    for(auto iter = monster->inventory()->begin(); iter != monster->inventory()->end(); ++iter)
    {
      std::cout << static_cast<int>(*iter) << " ";
    }
    std::cout << std::endl;

    std::cout << "monster color     : ";
    switch (monster->color())
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
