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
 * @file   eh5_meas.h
 * @brief  eCALHDF5 measurement class
**/

#pragma once

#include "eh5_meas.h"
#include <ecal/measurement/base/reader.h>

namespace eCAL
{
  namespace eh5
  {
 
    /**
 * @brief eCAL Reader API
**/
    class Reader : public measurement::base::Reader
    {
    public:
      /**
       * @brief Constructor
      **/
      Reader() = default;

      /**
       * @brief Constructor
       * 
       * @param path     Input file path / measurement directory path (see meas directory structure description bellow, in Open method).
       * @param access   Access type
       *
      **/
      explicit Reader(const std::string& path) : measurement(path, eAccessType::RDONLY){};
      
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
       bool Open(const std::string& path) override {
         return measurement.Open(path, eAccessType::RDONLY);
       }

      /**
       * @brief Close file
       *
       * @return         true if succeeds, false if it fails
      **/
       bool Close() override { return measurement.Close(); }

      /**
       * @brief Checks if file/Reader is ok
       *
       * @return  true if meas can be opened(read) false otherwise
      **/
       bool IsOk() const override { return measurement.IsOk(); }

      /**
       * @brief Get the File Type Version of the current opened file
       *
       * @return       file version
      **/
       std::string GetFileVersion() const override { return measurement.GetFileVersion(); }

      /**
       * @brief Gets maximum allowed size for an individual file
       *
       * @return       maximum size in MB
      **/
      /* size_t GetMaxSizePerFile() const override {}*/

      /**
       * @brief Get the available channel names of the current opened file / Reader
       *
       * @return       channel names
      **/
       std::set<std::string> GetChannelNames() const override { return measurement.GetChannelNames(); }

      /**
       * @brief Check if channel exists in Reader
       *
       * @param channel_name   name of the channel
       *
       * @return       true if exists, false otherwise
      **/
       bool HasChannel(const std::string& channel_name) const override { return measurement.HasChannel(channel_name);  }

      /**
       * @brief Get the channel description for the given channel
       *
       * @param channel_name  channel name
       *
       * @return              channel description
      **/
       std::string GetChannelDescription(const std::string& channel_name) const override { return measurement.GetChannelDescription(channel_name); }

      /**
       * @brief Gets the channel type of the given channel
       *
       * @param channel_name  channel name
       *
       * @return              channel type
      **/
       std::string GetChannelType(const std::string& channel_name) const override { return measurement.GetChannelType(channel_name); }

      /**
       * @brief Gets minimum timestamp for specified channel
       *
       * @param channel_name    channel name
       *
       * @return                minimum timestamp value
      **/
       long long GetMinTimestamp(const std::string& channel_name) const override { return measurement.GetMinTimestamp(channel_name); }

      /**
       * @brief Gets maximum timestamp for specified channel
       *
       * @param channel_name    channel name
       *
       * @return                maximum timestamp value
      **/
       long long GetMaxTimestamp(const std::string& channel_name) const override { return measurement.GetMaxTimestamp(channel_name); }

      /**
       * @brief Gets the header info for all data entries for the given channel
       *        Header = timestamp + entry id
       *
       * @param [in]  channel_name  channel name
       * @param [out] entries       header info for all data entries
       *
       * @return                    true if succeeds, false if it fails
      **/
       bool GetEntriesInfo(const std::string& channel_name, EntryInfoSet& entries) const override { return measurement.GetEntriesInfo(channel_name, entries); }

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
       bool GetEntriesInfoRange(const std::string& channel_name, long long begin, long long end, EntryInfoSet& entries) const override { return measurement.GetEntriesInfoRange(channel_name, begin, end, entries); }

      /**
       * @brief Gets data size of a specific entry
       *
       * @param [in]  entry_id   Entry ID
       * @param [out] size       Entry data size
       *
       * @return                 true if succeeds, false if it fails
      **/
       bool GetEntryDataSize(long long entry_id, size_t& size) const override { return measurement.GetEntryDataSize(entry_id, size); }

      /**
       * @brief Gets data from a specific entry
       *
       * @param [in]  entry_id   Entry ID
       * @param [out] data       Entry data
       *
       * @return                 true if succeeds, false if it fails
      **/
       bool GetEntryData(long long entry_id, void* data) const override { return measurement.GetEntryData(entry_id, data); }

    private:
      HDF5Meas measurement;

    };


  }  // namespace eh5
}  // namespace eCAL
