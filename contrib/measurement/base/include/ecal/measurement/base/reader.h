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
 * @file   Reader.h
 * @brief  Base class for low level Reader oprations
**/

#pragma once

#include <functional>
#include <set>
#include <string>
#include <memory>

#include <ecal/measurement/base/types.h>

namespace eCAL
{
  namespace measurement
  {
    namespace base
    {
      /**
       * @brief eCAL Reader API
      **/
      class Reader
      {
      public:
        /**
         * @brief Constructor
        **/
        Reader() = default;

        /**
         * @brief Destructor
        **/
        virtual ~Reader() = default;

        /**
         * @brief Copy operator
        **/
        Reader(const Reader& other) = delete;
        Reader& operator=(const Reader& other) = delete;

        /**
        * @brief Move operator
        **/
        Reader(Reader&&) = default;
        Reader& operator=(Reader&&) = default;

        /**
         * @brief Open file
         *
         * @param path     Input file path / Reader directory path.
         *
         *                 Default Reader directory structure:
         *                  - root directory e.g.: M:\Reader_directory\Reader01
         *                  - documents directory:                                |_doc
         *                  - hosts directories:                                  |_Host1 (e.g.: CARPC01)
         *                                                                        |_Host2 (e.g.: CARPC02)
         *
         *                 File path as input (AccessType::RDONLY):
         *                  - root directory (e.g.: M:\Reader_directory\Reader01) in this case all hosts subdirectories will be iterated,
         *                  - host directory (e.g.: M:\Reader_directory\Reader01\CARPC01),
         *                  - file path, path to file from Reader (e.g.: M:\Reader_directory\Reader01\CARPC01\meas01_05.hdf5).
         *
         *
         * @param access   Access type
         *
         * @return         true if input (AccessType::RDONLY) Reader/file path was opened, false otherwise.
        **/
        virtual bool Open(const std::string& path) = 0;

        /**
         * @brief Close file
         *
         * @return         true if succeeds, false if it fails
        **/
        virtual bool Close() = 0;

        /**
         * @brief Checks if file/Reader is ok
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
         * @brief Gets maximum allowed size for an individual file
         *
         * @return       maximum size in MB
        **/
        /*virtual size_t GetMaxSizePerFile() const = 0;*/

        /**
         * @brief Get the available channel names of the current opened file / Reader
         *
         * @return       channel names
        **/
        virtual std::set<std::string> GetChannelNames() const = 0;

        /**
         * @brief Check if channel exists in Reader
         *
         * @param channel_name   name of the channel
         *
         * @return       true if exists, false otherwise
        **/
        virtual bool HasChannel(const std::string& channel_name) const = 0;

        /**
         * @brief Get the channel description for the given channel
         *
         * @param channel_name  channel name
         *
         * @return              channel description
        **/
        virtual std::string GetChannelDescription(const std::string& channel_name) const = 0;

        /**
         * @brief Gets the channel type of the given channel
         *
         * @param channel_name  channel name
         *
         * @return              channel type
        **/
        virtual std::string GetChannelType(const std::string& channel_name) const = 0;

        /**
         * @brief Gets minimum timestamp for specified channel
         *
         * @param channel_name    channel name
         *
         * @return                minimum timestamp value
        **/
        virtual long long GetMinTimestamp(const std::string& channel_name) const = 0;

        /**
         * @brief Gets maximum timestamp for specified channel
         *
         * @param channel_name    channel name
         *
         * @return                maximum timestamp value
        **/
        virtual long long GetMaxTimestamp(const std::string& channel_name) const = 0;

        /**
         * @brief Gets the header info for all data entries for the given channel
         *        Header = timestamp + entry id
         *
         * @param [in]  channel_name  channel name
         * @param [out] entries       header info for all data entries
         *
         * @return                    true if succeeds, false if it fails
        **/
        virtual bool GetEntriesInfo(const std::string& channel_name, EntryInfoSet& entries) const = 0;

        /**
         * @brief Gets the header info for data entries for the given channel included in given time range (begin->end)
         *        Header = timestamp + entry id
         *
         * @param [in]  channel_name channel name
         * @param [in]  begin        time range begin timestamp
         * @param [in]  end          time range end timestamp
         * @param [out] entries      header info for data entries in given range
         *
         * @return                   true if succeeds, false if it fails
        **/
        virtual bool GetEntriesInfoRange(const std::string& channel_name, long long begin, long long end, EntryInfoSet& entries) const = 0;

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
  }  // namespace eh5
}  // namespace eCAL
