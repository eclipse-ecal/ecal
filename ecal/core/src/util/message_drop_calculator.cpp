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

void MessageDropCalculator::RegisterReceivedMessage(uint64_t received_message_counter) {
  if (!have_received_message) {
    // First-ever message or external → just initialize
    have_received_message = true;
    first_received_message_counter = received_message_counter;
  }

  last_received_message_counter = received_message_counter;
  ++total_messages_received;

  have_received_message_since_publisher_update = true;
}

void MessageDropCalculator::ApplyReceivedPublisherUpdate(uint64_t counter) {

  if (!have_received_publisher_message_counter)
  {
    have_received_publisher_message_counter = true;
    first_publisher_message_counter = counter;
  }

  last_publisher_message_counter = counter;

  have_received_message_since_publisher_update = false;
}

MessageDropCalculator::Summary MessageDropCalculator::GetSummary() {
  auto current_verified_drops = last_received_message_counter - first_received_message_counter - total_messages_received + 1;
  auto current_potential_drops = last_publisher_message_counter - first_publisher_message_counter - total_messages_received + 1;

  auto verified_drop_difference = current_verified_drops - verified_drops;
  auto potential_drop_difference = current_potential_drops - potential_drops;



}