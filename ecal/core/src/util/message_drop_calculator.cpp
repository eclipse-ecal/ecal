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
#include "message_drop_calculator.h"

using namespace eCAL;

void MessageDropCalculator::RegisterReceivedMessage(uint64_t received_message_counter) {
  if (!have_received_message) {
    // First-ever message or external → just initialize
    have_received_message = true;
    first_received_message_counter = received_message_counter;
  }

  last_received_message_counter = received_message_counter;
  ++total_messages_received;
}

MessageDropCalculator::Summary MessageDropCalculator::GetSummary() {
  Summary summary;

  if (have_received_message)
  {
    summary.drops = 1 + last_received_message_counter - first_received_message_counter - total_messages_received;
    summary.new_drops = (summary.drops > previous_number_of_drops);
    previous_number_of_drops = summary.drops;
  }

  return summary;
}