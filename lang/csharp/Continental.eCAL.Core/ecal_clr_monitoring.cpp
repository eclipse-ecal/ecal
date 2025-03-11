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

#include "ecal_clr_monitoring.h"
#include "ecal_clr_common.h"

#include <ecal/core.h>
#include <ecal/log.h>
#include <ecal/monitoring.h>

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

array<Byte>^ Monitoring::GetMonitoring(MonitoringEntity entities)
{
  std::string monitoring;
  ::eCAL::Monitoring::GetMonitoring(monitoring, static_cast<unsigned int>(entities));
  return StlStringToByteArray(monitoring);
}

array<Byte>^ Monitoring::GetLogging()
{
  std::string logging;
  ::eCAL::Logging::GetLogging(logging);
  return StlStringToByteArray(logging);
}
