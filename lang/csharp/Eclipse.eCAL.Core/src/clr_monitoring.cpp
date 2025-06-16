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

#include "clr_monitoring.h"
#include "clr_common.h"

#include <ecal/core.h>
#include <ecal/log.h>
#include <ecal/monitoring.h>

using namespace System;
using namespace Eclipse::eCAL::Core;
using namespace Internal;

array<System::Byte>^ Monitoring::GetSerializedMonitoring(MonitoringEntity entities)
{
  std::string monitoring_string;
  if (!::eCAL::Monitoring::GetMonitoring(monitoring_string, static_cast<unsigned int>(entities)))
  {
    return nullptr;
  }

  return StlStringToByteArray(monitoring_string);
}

SMonitoring^ Monitoring::GetMonitoring(MonitoringEntity entities)
{
  ::eCAL::Monitoring::SMonitoring native_monitoring;
  if (!::eCAL::Monitoring::GetMonitoring(native_monitoring, static_cast<unsigned int>(entities)))
  {
    return nullptr;
  }

  return gcnew SMonitoring(native_monitoring);
}
