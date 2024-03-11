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
#include <iostream>
#include <chrono>

int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "timer");

  std::cout << "eCAL Time Interface       : " << eCAL::Time::GetName()        << std::endl;
  std::cout << "eCAL Time is Synchronized : " << eCAL::Time::IsSynchronized() << std::endl;
  std::cout << "eCAL Time is Master       : " << eCAL::Time::IsMaster()       << std::endl;
  std::cout << std::endl;
  eCAL::Process::SleepMS(3000);

  while(eCAL::Ok())
  {
    auto now = eCAL::Time::ecal_clock::now();
    std::cout << "eCAL Time " << std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() << " ms" << std::endl;
    eCAL::Process::SleepMS(100);
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
