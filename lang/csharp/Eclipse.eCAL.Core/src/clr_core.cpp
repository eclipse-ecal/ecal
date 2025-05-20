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

#include "clr_core.h"
#include "clr_common.h"

#include <ecal/core.h>

using namespace Eclipse::eCAL::Core;
using namespace Internal;

System::String^ Core::GetVersion()
{
  return StlStringToString(::eCAL::GetVersionString());
}

System::String^ Core::GetDate()
{
  return StlStringToString(::eCAL::GetVersionDateString());
}

void Core::Initialize(System::String^ unitName)
{
  ::eCAL::Initialize(StringToStlString(unitName));
}

void Core::Initialize(System::String^ unitName, Init componentFlags)
{
  ::eCAL::Initialize(StringToStlString(unitName), static_cast<unsigned int>(componentFlags));
}

void Core::Initialize(Config::Configuration^ config, System::String^ unitName, Init componentFlags)
{
  auto native_config = config->ToNative();
  ::eCAL::Initialize(native_config, StringToStlString(unitName), static_cast<unsigned int>(componentFlags));
}

void Core::Terminate()
{
  ::eCAL::Finalize();
}

bool Core::IsInitialized()
{
  return ::eCAL::IsInitialized();
}

bool Core::Ok()
{
  return ::eCAL::Ok();
}
