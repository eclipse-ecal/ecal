/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
#include <ecal/types/custom_data_types.h>

#include <iostream>

int main()
{
  // creating config object
  eCAL::Configuration my_config;

  // setting a configuration
  my_config.registration.network_enabled = true;

  // initialize eCAL API
  eCAL::Initialize(my_config, "config sample");

  unsigned int counter = 0;
  // enter main loop
  while(eCAL::Ok())
  {
    // sleep 500 ms
    eCAL::Process::SleepMS(500);
    if (counter >= 10)
    {
      break;
    }

    std::cout << "Finished loop " << ++counter << "\n";
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}