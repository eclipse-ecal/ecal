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

#pragma once

#include <cstdint>
#include <mutex>
#include <map>
#include <bitset>

namespace eCAL
{
  /// \brief Calculates if any messages were not received by a subscriber (e.g. dropped)
///
/// Two update paths:
///  - registerReceivedMessage(seq): real messages — always counts missing IDs after the first.
///  - applyReceivedPublisherUpdate(seq): heartbeats — counts all missing IDs only if no real messages
///    arrived since the last external update.
///
/// Not thread-safe
class MessageDropCalculator {
public:
  /// \brief Snapshot of drop counts.
  /// \c newDrops is the count since the last getSummary() call;
  /// \c totalDrops is the count since construction.
  struct Summary {
    bool     new_drops = false; // new confirmed message drops since the last update
    uint64_t drops = 0;         // confirmed message drops
  };

  /// \brief Notify arrival of a real subscriber message.
  /// \param seq The sequence number received.
  void RegisterReceivedMessage(uint64_t received_message_counter);

  /// \brief Retrieve and reset the “newDrops” counter.
  /// \return A Summary of drops since last call and since construction.
  Summary GetSummary();

private:
  bool     have_received_message{ false };      ///< True once any registerReceivedMessage has ever run
  
  uint64_t first_received_message_counter{0};   ///< The counter of the first message that arrived
  uint64_t last_received_message_counter{0};    ///< The counter of the most recently received message
  uint64_t total_messages_received{ 0 };        ///< The total number of messages that have been received

  uint64_t previous_number_of_drops{ 0 };        ///< The number of detected drops, the last time that GetSummary was called
};

/// \brief Routes updates & summaries to per-key MessageDropCalculator instances,
///        and can also provide a cumulative summary over all keys.
///
/// Not thread-safe
template<typename Key>
class MessageDropCalculatorMap {
public:
  using Summary    = typename MessageDropCalculator::Summary;

  /// \brief Register a message counter that was received for a specific key
  void RegisterReceivedMessage(const Key& k, uint64_t message_counter);

  /// \brief Fetch summary for a specific key
  Summary GetSummary(const Key& k);

  /// \brief Fetch summary for a specific key aggregated over ALL keys
  std::map<Key, Summary> GetSummary();

private:
  std::map<Key, MessageDropCalculator> calculator_map_;
};

// Implementation of templated methods
template<typename Key>
void MessageDropCalculatorMap<Key>::RegisterReceivedMessage(const Key& k, uint64_t message_counter) {
  calculator_map_[k].RegisterReceivedMessage(message_counter);
}

template<typename Key>
typename MessageDropCalculatorMap<Key>::Summary
MessageDropCalculatorMap<Key>::GetSummary(const Key& k) {
  return calculator_map_[k].GetSummary();
}

template<typename Key>
std::map<Key, typename MessageDropCalculatorMap<Key>::Summary>
MessageDropCalculatorMap<Key>::GetSummary() {
  std::map<Key, typename MessageDropCalculatorMap<Key>::Summary> summary_map;
  for (auto& calculator : calculator_map_) {
    summary_map[calculator.first] = calculator.second.GetSummary();
  }
  return summary_map;
}

}