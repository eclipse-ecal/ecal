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

#include <string>
#include <tuple>
#include <ecalhdf5/eh5_types.h>

namespace eCAL
{
  namespace eh5
  {
    std::string GetEscapedTopicname(const std::string& input);
    std::string GetEscapedFilename(const std::string& input);
    std::string GetUnescapedString(const std::string& input);

    struct SEscapedChannel
    {
      std::string name;
      SChannel::id_t id;

      static SEscapedChannel fromSChannel(const SChannel& input)
      {
        return SEscapedChannel{ GetEscapedTopicname(input.name), input.id };
      }

      bool operator==(const SEscapedChannel& other) const
      {
        return std::tie(id, name) == std::tie(other.id, other.name);
      }

      bool operator!=(const SEscapedChannel& other) const
      {
        return !(*this == other);
      }

      bool operator<(const SEscapedChannel& other) const
      {
        return std::tie(id, name) < std::tie(other.id, other.name);
      }
    };

    struct SEscapedWriteEntry
    {
      // channel
      SEscapedChannel channel;

      // data
      const void* data = nullptr;
      unsigned long long size = 0;

      // metadata
      long long snd_timestamp = 0;
      long long rcv_timestamp = 0;
      long long sender_id = 0; // Unique ID which may be set by sender
      long long clock = 0;

      static SEscapedWriteEntry fromSWriteEntry(const SWriteEntry& input)
      {
        SEscapedWriteEntry escaped_entry;
        escaped_entry.channel = SEscapedChannel::fromSChannel(input.channel);
        escaped_entry.data = input.data;
        escaped_entry.size = input.size;
        escaped_entry.snd_timestamp = input.snd_timestamp;
        escaped_entry.rcv_timestamp = input.rcv_timestamp;
        escaped_entry.sender_id = input.sender_id;
        escaped_entry.clock = input.clock;
        return escaped_entry;
      }
    };
  }
}

namespace std {
  template <>
  struct hash<eCAL::eh5::SEscapedChannel> {
    std::size_t operator()(const eCAL::eh5::SEscapedChannel& data) const {
      // Combine the hash of the string and the integer
      std::size_t h1 = std::hash<std::string>{}(data.name);
      std::size_t h2 = std::hash<eCAL::experimental::measurement::base::Channel::id_t>{}(data.id);

      // Combine the two hashes (this is a common technique)
      return h1 ^ (h2 << 1); // XOR and shift
    }
  };
}
