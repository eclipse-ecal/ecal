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

#include "ecal_clr_time.h"
#include "ecal_clr_common.h"

#include <ecal/time.h>

using namespace Eclipse::eCAL::Time;
using namespace Internal;

String^ TimeAdapter::GetName()
{
  std::string name = ::eCAL::Time::GetName();
  return StlStringToString(name);
}

long long TimeAdapter::GetMicroSeconds()
{
  return ::eCAL::Time::GetMicroSeconds();
}

long long TimeAdapter::GetNanoSeconds()
{
  return ::eCAL::Time::GetNanoSeconds();
}

bool TimeAdapter::SetNanoSeconds(long long time)
{
  return ::eCAL::Time::SetNanoSeconds(time);
}

bool TimeAdapter::IsSynchronized()
{
  return ::eCAL::Time::IsSynchronized();
}

bool TimeAdapter::IsMaster()
{
  return ::eCAL::Time::IsMaster();
}

void TimeAdapter::SleepForNanoseconds(long long duration_nsecs)
{
  ::eCAL::Time::SleepForNanoseconds(duration_nsecs);
}

void TimeAdapter::GetStatus([System::Runtime::InteropServices::Out] int% error, [System::Runtime::InteropServices::Out] String^% statusMessage)
{
  int nativeError = 0;
  std::string nativeStatus;
  ::eCAL::Time::GetStatus(nativeError, &nativeStatus);
  error = nativeError;
  statusMessage = StlStringToString(nativeStatus);
}
