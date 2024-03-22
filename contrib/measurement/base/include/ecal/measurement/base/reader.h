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
#include <filesystem>

namespace eCAL
{
  namespace experimental
  {
    namespace measurement
    {
      namespace base
      {

        class MessageView;
        // visitor for each type (e.g. template based on data in the measurement... could be interesting)

        // When I have a measurement I would like to know what channels are in the measurement
        // i would like to find out more about the Channels, e.g. how many elements, timestamps of first & last element
        // i would like to be able to iterate over a measurement channel by channel
        // i would like to be able to iterate over all elements of a measurement




        class MessageView
        {
          class const_iterator;

          const_iterator begin();
          const_iterator end();
          const_iterator rbegin();
          const_iterator rend();


        };


        struct MessageMetaData
        {
          Channel channel;
        };

        using MessageFilter = std::function<bool(const MessageMetaData& metadata)>;

        class Reader {
        public:
          virtual ~Reader();

          virtual MessageView getMessageView() = 0;
          virtual MessageView getMessageView(const MessageFilter& filter);
          virtual MessageView getMessageView(const Channel& channel);

          std::set<Channel> GetChannels();
        };

        class DirectoryReader : Reader {



        };



        /**
         * @brief eCAL Measurement ReaderOld API
        **/
        class ReaderOld
        {
        public:
          /**
           * @brief Constructor
          **/
          ReaderOld() = default;

          /**
           * @brief Destructor
          **/
          virtual ~ReaderOld() = default;

          /**
           * @brief Copy operator
          **/
          ReaderOld(const ReaderOld& other) = delete;
          ReaderOld& operator=(const ReaderOld& other) = delete;

          /**
          * @brief Move operator
          **/
          ReaderOld(ReaderOld&&) = default;
          ReaderOld& operator=(ReaderOld&&) = default;

          /**
           * @brief Open file
           *
           * @param path     Input file path / ReaderOld directory path.
           *
           *                 Default ReaderOld directory structure:
           *                  - root directory e.g.: M:\ReaderOld_directory\ReaderOld01
           *                  - documents directory:                                |_doc
           *                  - hosts directories:                                  |_Host1 (e.g.: CARPC01)
           *                                                                        |_Host2 (e.g.: CARPC02)
           *
           *                 File path as input
           *                  - root directory (e.g.: M:\ReaderOld_directory\ReaderOld01) in this case all hosts subdirectories will be iterated,
           *                  - host directory (e.g.: M:\ReaderOld_directory\ReaderOld01\CARPC01),
           *                  - file path, path to file from ReaderOld (e.g.: M:\ReaderOld_directory\ReaderOld01\CARPC01\meas01_05.hdf5).
           *
           *
           * @return         true if input measurement/file path was opened, false otherwise.
          **/
          virtual bool Open(const std::string& path) = 0;

          /**
           * @brief Close file
           *
           * @return         true if succeeds, false if it fails
          **/
          virtual bool Close() = 0;

          /**
           * @brief Checks if file/measurement is ok
           *
           * @return  true if meas can be opened(read) false otherwise
          **/
          virtual bool IsOk() const = 0;

          /**
           * @brief Get the File Type Version of the current opened file
           *
           * @return       file version
          **/
          virtual std::string GetFileVersion() const = 0;

          /**
           * @brief Get the available channel names of the current opened file / measurement
           *
           * @return       channel names
          **/
          virtual std::set<std::string> GetChannelNames() const = 0;

          /**
           * @brief Get the available channel names of the current opened file / measurement
           *
           * @return Channels (channel name & id)
          **/
          virtual std::set<eCAL::experimental::measurement::base::Channel> GetChannels() const = 0;

          /**
           * @brief Get the available channel names of the current opened file / measurement
           *
           * @return Channels (channel name & id)
          **/
          virtual std::set<eCAL::experimental::measurement::base::Channel> GetChannels(const std::string& channel_name) const = 0;

          /**
           * @brief Check if channel exists in measurement
           *
           * @param channel  The channel (channel name & id)
           *
           * @return       true if exists, false otherwise
          **/
          virtual bool HasChannel(const eCAL::experimental::measurement::base::Channel& channel) const = 0;

          /**
           * @brief Get data type information of the given channel
           *
           * @param channel       (channel name & id)
           *
           * @return              channel type
          **/
          virtual DataTypeInformation GetChannelDataTypeInformation(const eCAL::experimental::measurement::base::Channel& channel) const = 0;

          /**
           * @brief Gets minimum timestamp for specified channel
           *
           * @param channel         (channel name & id)
           *
           * @return                minimum timestamp value
          **/
          virtual long long GetMinTimestamp(const eCAL::experimental::measurement::base::Channel& channel) const = 0;

          /**
           * @brief Gets maximum timestamp for specified channel
           *
           * @param channel         (channel name & id)
           *
           * @return                maximum timestamp value
          **/
          virtual long long GetMaxTimestamp(const eCAL::experimental::measurement::base::Channel & channel) const = 0;

          /**
           * @brief Gets the header info for all data entries for the given channel
           *        Header = timestamp + entry id
           *
           * @param [in]  channel       (channel name & id)
           * @param [out] entries       header info for all data entries
           *
           * @return                    true if succeeds, false if it fails
          **/
          virtual bool GetEntriesInfo(const eCAL::experimental::measurement::base::Channel & channel, EntryInfoSet& entries) const = 0;

          /**
           * @brief Gets the header info for data entries for the given channel included in given time range (begin->end)
           *        Header = timestamp + entry id
           *
           * @param [in]  channel      (channel name & id)
           * @param [in]  begin        time range begin timestamp
           * @param [in]  end          time range end timestamp
           * @param [out] entries      header info for data entries in given range
           *
           * @return                   true if succeeds, false if it fails
          **/
          virtual bool GetEntriesInfoRange(const eCAL::experimental::measurement::base::Channel& channel, long long begin, long long end, EntryInfoSet& entries) const = 0;

          /**
           * @brief Gets data size of a specific entry
           *
           * @param [in]  entry_id   Entry ID
           * @param [out] size       Entry data size
           *
           * @return                 true if succeeds, false if it fails
          **/
          virtual bool GetEntryDataSize(long long entry_id, size_t& size) const = 0;

          /**
           * @brief Gets data from a specific entry
           *
           * @param [in]  entry_id   Entry ID
           * @param [out] data       Entry data
           *
           * @return                 true if succeeds, false if it fails
          **/
          virtual bool GetEntryData(long long entry_id, void* data) const = 0;

        };
      }
    }
  }
}
