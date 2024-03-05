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

namespace eCAL
{
  namespace Logging
  {
    // compare two LogMessages for equality
    bool CompareLogMessages(const LogMessage& message1, const LogMessage& message2)
    {
      return (message1.time    == message2.time &&
              message1.hname   == message2.hname &&
              message1.pid     == message2.pid &&
              message1.pname   == message2.pname &&
              message1.uname   == message2.uname &&
              message1.level   == message2.level &&
              message1.content == message2.content);
    }
  }
}
