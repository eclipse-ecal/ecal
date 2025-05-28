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

#include "clr_process.h"
#include "clr_common.h"

using namespace System;
using namespace msclr::interop;

namespace Eclipse {
  namespace eCAL {
    namespace Core {

      void Process::DumpConfig()
      {
        ::eCAL::Process::DumpConfig();
      }

      String^ Process::DumpConfigToString()
      {
        std::string cfg;
        ::eCAL::Process::DumpConfig(cfg);
        return Internal::StlStringToString(cfg);
      }

      String^ Process::GetHostName()
      {
        return Internal::StlStringToString(::eCAL::Process::GetHostName());
      }

      String^ Process::GetShmTransportDomain()
      {
        return Internal::StlStringToString(::eCAL::Process::GetShmTransportDomain());
      }

      String^ Process::GetUnitName()
      {
        return Internal::StlStringToString(::eCAL::Process::GetUnitName());
      }

      void Process::SleepMS(long time_ms)
      {
        ::eCAL::Process::SleepMS(time_ms);
      }

      void Process::SleepNS(long long time_ns)
      {
        ::eCAL::Process::SleepNS(time_ns);
      }

      int Process::GetProcessID()
      {
        return ::eCAL::Process::GetProcessID();
      }

      String^ Process::GetProcessIDAsString()
      {
        return Internal::StlStringToString(::eCAL::Process::GetProcessIDAsString());
      }

      String^ Process::GetProcessName()
      {
        return Internal::StlStringToString(::eCAL::Process::GetProcessName());
      }

      String^ Process::GetProcessParameter()
      {
        return Internal::StlStringToString(::eCAL::Process::GetProcessParameter());
      }

      void Process::SetState(eProcessSeverity severity, eProcessSeverityLevel level, String^ info)
      {
        ::eCAL::Process::SetState(
          ProcessSeverityHelper::ToNative(severity),
          ProcessSeverityLevelHelper::ToNative(level),
          Internal::StringToStlString(info)
        );
      }

    } // namespace Core
  } // namespace eCAL
} // namespace Eclipse