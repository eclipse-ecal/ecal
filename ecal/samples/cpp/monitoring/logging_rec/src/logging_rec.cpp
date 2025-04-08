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

#include <iostream>

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <ecal/core/pb/logging.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

int main()
{
  // switch udp logging receiver on
  eCAL::Configuration configuration;
  configuration.logging.receiver.enable = true;

  // initialize eCAL core API
  eCAL::Initialize(configuration, "logging");

  // logging instance to store snapshot
  eCAL::pb::LogMessageList logmessage_list;
  std::string              logmessage_list_s;

  // monitor for ever
  while (eCAL::Ok())
  {
    // take snapshot
    eCAL::Logging::GetLogging(logmessage_list_s);
    logmessage_list.ParseFromString(logmessage_list_s);

    for (auto logmessage : logmessage_list.log_messages())
    {
      std::cout << "Time        : " << logmessage.time() << "\n";
      std::cout << "HostName    : " << logmessage.host_name() << "\n";
      std::cout << "ProcessId   : " << logmessage.process_id() << "\n";
      std::cout << "ProcessName : " << logmessage.process_name() << "\n";
      std::cout << "UnitName    : " << logmessage.unit_name() << "\n";
      std::cout << "Level       : " << logmessage.level() << "\n";
      std::cout << "Content     : " << logmessage.content() << "\n";
      std::cout << "\n";
    }

    // sleep 1 second
    eCAL::Process::SleepMS(1000);
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
