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
 * @file  ecal_clr_monitoring.cpp
**/

#include "ecal_clr_monitoring.h"
#include "ecal_clr_common.h"

#include <ecal/ecal.h>

using namespace Continental::eCAL::Core;
using namespace Internal;

void Monitoring::Initialize()
{
  // Initialize eCAL with the monitoring component only.
  ::eCAL::Initialize("", ::eCAL::Init::Monitoring);
}

void Monitoring::Terminate()
{
  ::eCAL::Finalize();
}

String^ Monitoring::GetMonitoring(MonitoringEntity entities)
{
  std::string monitoring;
  ::eCAL::Monitoring::GetMonitoring(monitoring, static_cast<unsigned int>(entities));
  return StlStringToString(monitoring);
}

String^ Monitoring::GetLogging()
{
  std::string logging;
  ::eCAL::Logging::GetLogging(logging);
  return StlStringToString(logging);
}

array<Byte>^ Monitoring::GetMonitoringBytes(MonitoringEntity entities)
{
  std::string monitoring;
  ::eCAL::Monitoring::GetMonitoring(monitoring, static_cast<unsigned int>(entities));
  array<Byte>^ data = gcnew array<Byte>(static_cast<int>(monitoring.size()));
  System::Runtime::InteropServices::Marshal::Copy(IntPtr((void*)monitoring.data()), data, 0, static_cast<int>(monitoring.size()));
  return data;
}

array<Byte>^ Monitoring::GetLoggingBytes()
{
  std::string logging;
  ::eCAL::Logging::GetLogging(logging);
  array<Byte>^ data = gcnew array<Byte>(static_cast<int>(logging.size()));
  System::Runtime::InteropServices::Marshal::Copy(IntPtr((void*)logging.data()), data, 0, static_cast<int>(logging.size()));
  return data;
}
