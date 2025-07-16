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
#include <unordered_map>
#include <bitset>

namespace ecal
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
    bool newDrops;
    uint64_t totalDrops;
  };

  /// \brief Notify arrival of a real subscriber message.
  /// \param seq The sequence number received.
  void RegisterReceivedMessage(uint64_t seq);

  /// \brief Notify arrival of a publisher heartbeat.
  /// \param seq The latest publisher sequence number.
  void ApplyReceivedPublisherUpdate(uint64_t seq);

  /// \brief Retrieve and reset the “newDrops” counter.
  /// \return A Summary of drops since last call and since construction.
  Summary GetSummary();

  bool     have_received_message{ false };      ///< True once any registerReceivedMessage has ever run
  uint64_t first_received_message_counter{0};   ///<
  uint64_t last_received_message_counter{0};    ///< Last seen sequence
  uint64_t total_messages_received{ 0 };        /// 

  bool     have_received_message_since_publisher_update{ false };

  uint64_t have_received_publisher_message_counter{ 0 }; ///< We need to skip the first few updates, before counting as drops
  uint64_t first_publisher_message_counter{0};           ///< 
  uint64_t last_publisher_message_counter{0};            ///< Last external update

  uint64_t verified_drops{0};               ///< Accumulated drop count
  uint64_t potential_drops{0};              ///< verified_drops at last getSummary()
};



/// \brief Routes updates & summaries to per-key MessageDropCalculator instances,
///        and can also provide a cumulative summary over all keys.
///
/// Thread-safe for concurrent calls.
template<typename Key>
class MessageDropCalculatorMap {
public:
  using Calculator = MessageDropCalculator;
  using Summary    = typename Calculator::Summary;

  /// \brief Register a message counter that was received for a specific key
  void RegisterReceivedMessage(const Key& k, uint64_t message_counter);

  /// \brief External-heartbeat update for a given key
  void ApplyReceivedPublisherUpdate(const Key& k, uint64_t expected_message_counter);

  /// \brief Fetch summary for a specific key
  Summary GetSummary(const Key& k);

  /// \brief Fetch summary for a specific key aggregated over ALL keys
  std::map<Key, Summary> GetSummary();

private:
  std::unordered_map<Key, Calculator> calculator_map_;
};

// Implementation of templated methods
template<typename Key>
void MessageDropCalculatorMap<Key>::RegisterReceivedMessage(const Key& k, uint64_t message_counter) {
  calculator_map_[k].RegisterReceivedMessage(message_counter);
}

template<typename Key>
void MessageDropCalculatorMap<Key>::ApplyReceivedPublisherUpdate(const Key& k, uint64_t expected_message_counter) {
  calculator_map_[k].ApplyReceivedPublisherUpdate(expected_message_counter);
}

template<typename Key>
typename MessageDropCalculatorMap<Key>::Summary
MessageDropCalculatorMap<Key>::GetSummary(const Key& k) {
  calculator_map_[k].GetSummary();
}

template<typename Key>
typename MessageDropCalculatorMap<Key>::Summary
MessageDropCalculatorMap<Key>::GetSummary() {
  Summary aggregated_summary{0,0};
  for (auto &calculator : calculator_map_) {
    auto s = calculator.second.getSummary();
    aggregated_summary.newDrops   != s.newDrops;
    aggregated_summary.totalDrops += s.totalDrops;
  }
  return aggregated_summary;
}

}