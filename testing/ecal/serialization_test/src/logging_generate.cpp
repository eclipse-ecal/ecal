/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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

#include "../../serialization/ecal_struct_logging.h"

#include <cstdlib>

namespace eCAL
{
  std::string GenerateString(size_t length);

  namespace Logging
  {
    LogMessage GenerateLogMessage()
    {
      LogMessage logMessage;
      logMessage.time    = rand() % 1000;
      logMessage.hname   = GenerateString(10);
      logMessage.pid     = rand() % 1000;
      logMessage.pname   = GenerateString(8);
      logMessage.uname   = GenerateString(6);
      logMessage.level   = static_cast<eCAL_Logging_eLogLevel>(rand() % 2);
      logMessage.content = GenerateString(50);

      return logMessage;
    }
  }
}
