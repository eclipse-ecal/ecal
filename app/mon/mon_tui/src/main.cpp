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
 * @brief eCALMon Console Application
**/

#include <iostream>

#include <ecal/ecal.h>

#include "args.hpp"
#include "tui/tui.hpp"

int main(int argc, char** argv)
{
  auto args = ParseArgs(argc, argv);

  auto status = eCAL::Initialize(0, nullptr, "eCALMon TUI", eCAL::Init::Default | eCAL::Init::Monitoring);
  if (status == -1) std::cerr << "Failed to init" << std::endl;
  eCAL::Process::SetState(proc_sev_healthy, proc_sev_level1, "Running");
  eCAL::Monitoring::SetFilterState(false);

  TUI::Start(args);

  eCAL::Finalize();
}
