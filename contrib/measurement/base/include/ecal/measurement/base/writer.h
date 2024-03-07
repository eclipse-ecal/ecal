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
#include <set>
#include <string>
#include <memory>
#include <optional>

#include <ecal/measurement/base/types.h>

namespace eCAL
{
  namespace experimental
  {
    namespace measurement
    {
      namespace base
      {
        struct FileOptions
        {
          std::string path = "";
          std::string base_name = "";
        };

        using ChannelSplittingOptions = bool;
        using SizeSplittingOptions = std::optional<uint64_t>;


        struct WriterOptions
        {
          FileOptions             file_options;
          SizeSplittingOptions    size_splitting = 512 * 1024 * 1024;
          ChannelSplittingOptions channel_splitting = false;
        };

        enum class WritterErrorCode
        {
          FileCreationFailure = 0,  // There was a problem creating a new file
          AddEntryFailure = 1,      // The entry could not be added to the file (for whatever reason)
          NoWriterFailure = 2       // This writer interface does not contain a writer any more
        };

        struct WriterError
        {
          WritterErrorCode code;
          std::string message;
        };

        struct WriterEntry
        {
          void* data = nullptr;
          unsigned long long size = 0;
          long long snd_timestamp = 0;
          long long rcv_timestamp = 0;
          long long clock;
        };

        /**
         * @brief eCAL Measurement Writer API
        **/
        class Writer
        {
        public:
          /**
           * @brief Constructor
          **/
          Writer() = default;

          /**
           * @brief Destructor
          **/
          virtual ~Writer() = default;

          /**
           * @brief Copy operator
          **/
          Writer(const Writer& other) = delete;
          Writer& operator=(const Writer& other) = delete;

          /**
          * @brief Move operator
          **/
          Writer(Writer&&) = default;
          Writer& operator=(Writer&&) = default;

          /**
           * @brief Set data type information of the given channel
           *
           * @param channel_name  channel name
           * @param info          datatype info of the channel
           *
           * @return              channel type
          **/
          virtual void SetChannelDataTypeInformation(const eCAL::experimental::measurement::base::Channel& channel, const DataTypeInformation& info) = 0;

          /**
           * @brief Add entry to file
           *
           * @param data           data to be added
           * @param size           size of the data
           * @param snd_timestamp  send time stamp
           * @param rcv_timestamp  receive time stamp
           * @param channel_name   channel name
           * @param id             message id
           * @param clock          message clock
           *
           * @return              true if succeeds, false if it fails
          **/
          virtual void AddEntryToFile(const eCAL::experimental::measurement::base::Channel& channel, const WriterEntry& entry) = 0;
        };

        using WriterCreator = std::function < std::unique_ptr<Writer>(const FileOptions& options)>;
      }
    }
  }
}
