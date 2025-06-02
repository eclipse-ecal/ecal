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
      String^ Process::GetUnitName()
      {
        return Internal::StlStringToString(::eCAL::Process::GetUnitName());
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