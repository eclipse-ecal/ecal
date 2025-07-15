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

using namespace ecal;

void MessageDropCalculator::registerReceivedMessage(uint64_t received_message_counter) {
  if (!have_received_message) {
    // First-ever message or external → just initialize
    have_received_message = true;
    last_received_message_counter = received_message_counter;
  }
  else
  {
    // Message counters should be sequential. If the counter increased by more than one, those are message drops.
    uint64_t diff = forwardDistance(received_message_counter, last_received_message_counter);
    if (diff > 0)
    {
      verified_drops += diff - 1;
    }
  }
  have_received_message_since_publisher_update = true;
}

void MessageDropCalculator::applyReceivedPublisherUpdate(uint64_t counter) {
  if (received_publisher_updates >= 2)
  {
    uint64_t diff = [&]() {
      if (have_received_message)
      {
        return forwardDistance(counter, last_received_message_counter);
      }
      else
      {
        return counter;
      }

    }();

    if (diff > 0 && have_received_message_since_publisher_update) {
      // Count full diff when no real messages intervened
      potential_drops = diff;
    }
  }

  last_publisher_message_counter = counter;
  ++received_publisher_updates;
  have_received_message_since_publisher_update = false;
}

MessageDropCalculator::Summary MessageDropCalculator::getSummary() {
  uint64_t all    = verified_drops + potential_drops;
  return { false, all };
}