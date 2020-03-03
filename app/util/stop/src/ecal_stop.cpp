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

/**
 * @brief eCALStop Console Application
**/

#include <algorithm>
#include <iostream>

#include <ecal/ecal.h>

int main(int argc, char** argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "eCALStop", eCAL::Init::All);

  // set process state
  eCAL::Process::SetState(proc_sev_healthy, proc_sev_level1, "Running");

  // some nice info show while collection
  std::cout << "Collecting local process informations." << std::endl;
  std::string sstring = "\nPlease wait ..........................\nStand by .............................\nStay tuned ...........................\n\n";
  std::reverse(sstring.begin(), sstring.end());
  while (!sstring.empty())
  {
    std::cout << sstring.back();
    sstring.pop_back();
    eCAL::Process::SleepMS(25);
  }
  eCAL::Process::SleepMS(500);

  // shut down local user processes
  std::cout << "--------------------------------------" << std::endl;
  std::cout << "Shutdown local eCAL user processes." << std::endl;
  std::cout << "--------------------------------------" << std::endl;
  eCAL::Util::ShutdownProcesses();
  std::cout << std::endl;

  // shut down local eCAL core
  std::cout << "--------------------------------------" << std::endl;
  std::cout << "Shutdown local eCAL core components." << std::endl;
  std::cout << "--------------------------------------" << std::endl;
  eCAL::Util::ShutdownCore();
  std::cout << std::endl;

  // finalize eCAL API
  eCAL::Finalize();

  return EXIT_SUCCESS;
}
