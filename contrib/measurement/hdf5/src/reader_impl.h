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

/**
 * @file   reader.h
 * @brief  Base class for low level measurement reading operations
**/

#pragma once

#include <functional>
#include <set>
#include <string>
#include <memory>

#include <ecal/measurement/base/types.h>

namespace eCAL
{
  namespace experimental
  {
    namespace measurement
    {
      namespace base
      {
        class ReaderOld
        {
        public:
          ReaderOld() = default;
          virtual ~ReaderOld() = default;

          ReaderOld(const ReaderOld& other) = delete;
          ReaderOld& operator=(const ReaderOld& other) = delete;

          ReaderOld(ReaderOld&&) = default;
          ReaderOld& operator=(ReaderOld&&) = default;

          // Only for FileReaders
          //virtual std::string GetFileVersion() const = 0;

          virtual std::set<std::string> GetChannelNames() const = 0;

          virtual std::set<eCAL::experimental::measurement::base::Channel> GetChannels() const = 0;

          virtual std::set<eCAL::experimental::measurement::base::Channel> GetChannels(const std::string& channel_name) const = 0;

          virtual bool HasChannel(const eCAL::experimental::measurement::base::Channel& channel) const = 0;

          virtual DataTypeInformation GetChannelDataTypeInformation(const eCAL::experimental::measurement::base::Channel& channel) const = 0;

          virtual long long GetMinTimestamp(const eCAL::experimental::measurement::base::Channel& channel) const = 0;

          virtual long long GetMaxTimestamp(const eCAL::experimental::measurement::base::Channel& channel) const = 0;

          virtual bool GetEntriesInfo(const eCAL::experimental::measurement::base::Channel& channel, EntryInfoSet& entries) const = 0;

          virtual bool GetEntriesInfoRange(const eCAL::experimental::measurement::base::Channel& channel, long long begin, long long end, EntryInfoSet& entries) const = 0;

          virtual bool GetEntryDataSize(long long entry_id, size_t& size) const = 0;

          virtual bool GetEntryData(long long entry_id, void* data) const = 0;

        };
      }
    }
  }
}
