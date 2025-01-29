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

#include "logging_compare.h"

namespace eCAL
{
  namespace Logging
  {
    // compare two LogMessages for equality
    bool CompareLogMessages(const SLogMessage& message1, const SLogMessage& message2)
    {
      return (message1.time         == message2.time &&
              message1.host_name    == message2.host_name &&
              message1.process_id   == message2.process_id &&
              message1.process_name == message2.process_name &&
              message1.unit_name    == message2.unit_name &&
              message1.level        == message2.level &&
              message1.content      == message2.content);
    }
  }
}
