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

/**
 * @file   eh5_reader.h
 * @brief  Hdf5 based Reader implementation
**/

#pragma once

#include <memory>
#include <ecal/measurement/base/reader.h>

namespace eCAL
{
  namespace experimental
  {
    namespace measurement
    {
      namespace hdf5
      {
        struct ReaderImpl;

        /**
         * @brief Hdf5 based Reader Implementation
        **/
        class Reader : public measurement::base::Reader
        {
        public:
          /**
           * @brief Constructor
          **/
          Reader();

          /**
           * @brief Constructor
           *
           * @param path     Input file path / measurement directory path (see meas directory structure description bellow, in Open method).
           *
          **/
          explicit Reader(const std::string& path);

          /**
           * @brief Destructor
          **/
          virtual ~Reader();

          /**
           * @brief Copy operator
          **/
          Reader(const Reader& other) = delete;
          Reader& operator=(const Reader& other) = delete;

          /**
          * @brief Move operator
          **/
          Reader(Reader&&) noexcept;
          Reader& operator=(Reader&&) noexcept;

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
           *                 File path as input
           *                  - root directory (e.g.: M:\Reader_directory\Reader01) in this case all hosts subdirectories will be iterated,
           *                  - host directory (e.g.: M:\Reader_directory\Reader01\CARPC01),
           *                  - file path, path to file from Reader (e.g.: M:\Reader_directory\Reader01\CARPC01\meas01_05.hdf5).
           *
           *
           * @return         true if input measurement/file path was opened, false otherwise.
          **/
          bool Open(const std::string& path) override;

          /**
           * @brief Close file
           *
           * @return         true if succeeds, false if it fails
          **/
          bool Close() override;

          /**
           * @brief Checks if file/measurement is ok
           *
           * @return  true if meas can be opened(read) false otherwise
          **/
          bool IsOk() const override;

          /**
           * @brief Get the File Type Version of the current opened file
           *
           * @return       file version
          **/
          std::string GetFileVersion() const override;

          /**
           * @brief Get the available channel names of the current opened file / measurement
           *
           * @return       channel names
          **/
          //std::set<std::string> GetChannelNames() const override;

          /**
           * @brief Get the available channel names of the current opened file / measurement
           *
           * @return Channels (channel name & id)
          **/
          std::set<eCAL::experimental::measurement::base::Channel> GetChannels() const override;

          /**
           * @brief Get the available channel names of the current opened file / measurement
           *
           * @return Channels (channel name & id)
          **/
          //std::set<eCAL::experimental::measurement::base::Channel> GetChannels(const std::string& channel_name) const override;

          /**
           * @brief Check if channel exists in measurement
           *
           * @param channel_name   name of the channel
           *
           * @return       true if exists, false otherwise
          **/
          bool HasChannel(const eCAL::experimental::measurement::base::Channel& channel) const override;

          /**
           * @brief Get data type information of the given channel
           *
           * @param channel_name  channel name
           *
           * @return              channel type
          **/
          base::DataTypeInformation GetChannelDataTypeInformation(const eCAL::experimental::measurement::base::Channel& channel) const override;

          /**
           * @brief Gets minimum timestamp for specified channel
           *
           * @param channel_name    channel name
           *
           * @return                minimum timestamp value
          **/
          long long GetMinTimestamp(const eCAL::experimental::measurement::base::Channel& channel) const override;

          /**
           * @brief Gets maximum timestamp for specified channel
           *
           * @param channel_name    channel name
           *
           * @return                maximum timestamp value
          **/
          long long GetMaxTimestamp(const eCAL::experimental::measurement::base::Channel& channel) const override;

          /**
           * @brief Gets the header info for all data entries for the given channel
           *        Header = timestamp + entry id
           *
           * @param [in]  channel_name  channel name
           * @param [out] entries       header info for all data entries
           *
           * @return                    true if succeeds, false if it fails
          **/
          bool GetEntriesInfo(const eCAL::experimental::measurement::base::Channel& channel, measurement::base::EntryInfoSet& entries) const override;

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
          bool GetEntriesInfoRange(const eCAL::experimental::measurement::base::Channel& channel, long long begin, long long end, measurement::base::EntryInfoSet& entries) const override;

          /**
           * @brief Gets data size of a specific entry
           *
           * @param [in]  entry_id   Entry ID
           * @param [out] size       Entry data size
           *
           * @return                 true if succeeds, false if it fails
          **/
          bool GetEntryDataSize(long long entry_id, size_t& size) const override;

          /**
           * @brief Gets data from a specific entry
           *
           * @param [in]  entry_id   Entry ID
           * @param [out] data       Entry data
           *
           * @return                 true if succeeds, false if it fails
          **/
          bool GetEntryData(long long entry_id, void* data) const override;

          /**
          * @brief Gets data from a specific entry and stores it in a std::string
          *        Can be used to speed up retrieval, as underlying datasets need to be opened only once.
          *
          * @param [in]  entry_id   Entry ID
          * @param [out]  data      Data to be  read  from the measurement
          * @return                 Data was retrieved successfully
          **/
          bool GetEntryDataAsString(long long entry_id, std::string& data) const override;

        private:
          std::unique_ptr<ReaderImpl> impl;

        };

      }  //namespace hdf5
    }  // namespace measurement
  }  // namespace experimental
}  // namespace eCAL
