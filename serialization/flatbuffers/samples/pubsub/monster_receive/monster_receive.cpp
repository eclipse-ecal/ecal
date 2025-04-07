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
#include <ecal/msg/flatbuffers/subscriber.h>

#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>

// flatbuffers includes
#include <flatbuffers/flatbuffers.h>

// flatbuffers generated includes
#include <monster/monster_generated.h>


void OnFlatMonster(const eCAL::STopicId& topic_id_, const Game::Sample::Monster* const & monster_, long long time_, long long /*clock_*/)
{
  // print content
  std::cout << "topic name        : " << topic_id_.topic_name      << std::endl;
  std::cout << "time              : " << time_                     << std::endl;
  std::cout                                                        << std::endl;
  std::cout << "monster pos x     : " << monster_->pos()->x()       << std::endl;
  std::cout << "monster pos y     : " << monster_->pos()->y()       << std::endl;
  std::cout << "monster pos z     : " << monster_->pos()->z()       << std::endl;
  std::cout << "monster mana      : " << monster_->mana()           << std::endl;
  std::cout << "monster hp        : " << monster_->hp()             << std::endl;
  std::cout << "monster name      : " << monster_->name()->c_str()  << std::endl;

  std::cout << "monster inventory : ";
  for(auto iter = monster_->inventory()->begin(); iter != monster_->inventory()->end(); ++iter)
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

void OnObjectMonster(const eCAL::STopicId& topic_id_, const Game::Sample::MonsterT* const& monster_, long long time_, long long /*clock_*/)
{
  // print content
  std::cout << "topic name        : " << topic_id_.topic_name << std::endl;
  std::cout << "time              : " << time_ << std::endl;
  std::cout << std::endl;
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
  // initialize eCAL API
  eCAL::Initialize("monster_receive");

  // set process state
  eCAL::Process::SetState(eCAL::Process::eSeverity::healthy, eCAL::Process::eSeverityLevel::level1, "I feel good !");

  // There are two different types of Flatbuffer subscribers
  // One type is a "flat" type, e.g. it uses the API which is backed by raw memory
  // The other type is an "object" type, where the type is directly mapped to e.g. std::string / std::vector ... types.

  eCAL::flatbuffers::CFlatSubscriber<Game::Sample::Monster> flat_subscriber("monster");
  flat_subscriber.SetReceiveCallback(OnFlatMonster);

  eCAL::flatbuffers::CObjectSubscriber<Game::Sample::MonsterT> object_subscriber("monster");
  object_subscriber.SetReceiveCallback(OnObjectMonster);

  while(eCAL::Ok())
  {
    // sleep 100 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // finalize eCAL API
  eCAL::Finalize();
}
