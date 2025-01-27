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

#ifdef ECAL_OS_WINDOWS
const char* proc_name = "notepad.exe";
#else  // ECAL_OS_WINDOWS
const char* proc_name = "gedit";
#endif // ECAL_OS_WINDOWS

int main()
{
  // initialize eCAL API
  eCAL::Initialize("process", eCAL::Init::None);

  // start process
  eCAL::Process::StartProcess(proc_name, "", "", false, eCAL::Process::eStartMode::normal, false);

  // sleep 2 seconds
  eCAL::Process::SleepMS(2000);

  // stop process
  eCAL::Process::StopProcess(proc_name);

  // sleep 2 seconds
  eCAL::Process::SleepMS(2000);

  // start process
  int process_id = eCAL::Process::StartProcess(proc_name, "", "", false, eCAL::Process::eStartMode::normal, false);

  // sleep 2 seconds
  eCAL::Process::SleepMS(2000);

  // stop notepad
  eCAL::Process::StopProcess(process_id);

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
