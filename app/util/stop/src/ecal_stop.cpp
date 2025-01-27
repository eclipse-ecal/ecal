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

/**
 * @brief eCALStop Console Application
**/

#include <algorithm>
#include <iostream>

#include <ecal/ecal.h>

int main()
{
  // initialize eCAL API
  eCAL::Initialize("eCALStop", eCAL::Init::All);

  // set process state
  eCAL::Process::SetState(eCAL::Process::eSeverity::healthy, eCAL::Process::eSeverityLevel::level1, "Running");

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

  eCAL::Monitoring::SMonitoring monitoring;
  eCAL::Monitoring::GetMonitoring(monitoring, eCAL::Monitoring::Entity::Process);
  const std::string host_name(eCAL::Process::GetHostName());

  for (const auto& process : monitoring.processes)
  {
    // filter out eCAL system processes
    const std::string unit_name = process.unit_name;
    if ( (unit_name != "eCALConfig")
      && (unit_name != "eCALMon")
      && (unit_name != "eCALMon CLI")
      && (unit_name != "eCALMon TUI")
      && (unit_name != "eCALPlay")
      && (unit_name != "eCALPlayGUI")
      && (unit_name != "eCALRec")
      && (unit_name != "eCALRecGUI")
      && (unit_name != "eCALRecClient")
      && (unit_name != "eCALRec-Remote")
      && (unit_name != "eCALRec-Server")
      && (unit_name != "eCALSys")
      && (unit_name != "eCALSysGUI")
      && (unit_name != "eCALSysClient")
      && (unit_name != "eCALSys-Remote")
      && (unit_name != "eCALStop")
      && (process.host_name == host_name)
      )
    {
      std::cout << "Stopping process " << process.process_name << " (" << process.process_id << ")" << std::endl;
      eCAL::Util::ShutdownProcess(process.process_id);
    }
  }

  // finalize eCAL API
  eCAL::Finalize();

  return EXIT_SUCCESS;
}
