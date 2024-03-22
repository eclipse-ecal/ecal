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
 * @file   writer.h
 * @brief  Base class for low level measurement writing operations
**/

#pragma once

#include <functional>
#include <map>
#include <set>
#include <string>
#include <memory>
#include <optional>

#include <ecal/measurement/base/writer.h>

#include <hdf5.h>
#include <hdf5_helper.h>

namespace eCAL
{
  namespace experimental
  {
    namespace measurement
    {
      namespace hdf5
      {
        /**
         * @brief eCAL Measurement Writer API
        **/
        class V6Writer : public base::Writer
        {
        public:
          V6Writer(const base::FileOptions& file_options);
          ~V6Writer() noexcept;

          V6Writer(const V6Writer& other) = delete;
          V6Writer& operator=(const V6Writer& other) = delete;

          V6Writer(V6Writer&&) = delete;
          V6Writer& operator=(V6Writer&&) = delete;

          void SetChannelDataTypeInformation(const base::Channel& channel, const base::DataTypeInformation& info) override;
          void AddEntryToFile(const eCAL::experimental::measurement::base::Channel& channel, const base::WriterEntry& entry) override;

          static base::WriterCreator GetCreator();
        private:
          std::string CreateFilename(const base::FileOptions& file_options);
          base::EntryInfo CreateEntryInfo(const eCAL::experimental::measurement::base::Channel& channel, const base::WriterEntry& entry) const;
          void WriteChannelSummary();

          hid_t root = 0;
          std::map<base::Channel, ChannelSummary> channel_summaries;
          long long current_id = 0;
        };
      }
    }
  }
}