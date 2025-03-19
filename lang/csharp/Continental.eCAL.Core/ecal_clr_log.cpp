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

#include "ecal_clr_log.h"
#include "ecal_clr_common.h"

#include <ecal/log.h>

using namespace Continental::eCAL::Core;
using namespace Internal;

void Logging::Log(LogLevel level, System::String^ message)
{
  // Convert the managed string to a native std::string
  std::string nativeMsg = StringToStlString(message);
  
  // Cast the managed LogLevel to the native eCAL::Logging::eLogLevel.
  // The values match, so a static_cast is sufficient.
  ::eCAL::Logging::Log(static_cast<::eCAL::Logging::eLogLevel>(level), nativeMsg);
}

array<Byte>^ Logging::GetLogging()
{
  std::string logging;
  ::eCAL::Logging::GetLogging(logging);
  return StlStringToByteArray(logging);
}
