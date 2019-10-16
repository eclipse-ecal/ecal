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

#include <functional>
#include <set>
#include <string>

#include "eh5_types.h"

namespace eCAL
{
  namespace eh5
  {
    class HDF5MeasImpl;

    /**
     * @brief eCAL HDF5 measurement API
    **/
    class HDF5Meas
    {
    public:
      /**
       * @brief Constructor
      **/
      HDF5Meas();

      /**
       * @brief Constructor
       *
       * @param path     Input file path / measurement directory path (see meas directory structure description bellow, in Open method).
       * @param access   Access type
       *
      **/
      explicit HDF5Meas(const std::string& path, eAccessType access = eAccessType::RDONLY);

      /**
       * @brief Destructor
      **/
      ~HDF5Meas();

      /**
       * @brief Open file
       *
       * @param path     Input file path / measurement directory path.
       *
       *                 Default measurement directory structure:
       *                  - root directory e.g.: M:\measurement_directory\measurement01
       *                  - documents directory:                                |_doc
       *                  - hosts directories:                                  |_Host1 (e.g.: CARPC01)
       *                                                                        |_Host2 (e.g.: CARPC02)
       *
       *                 File path as input (eAccessType::RDONLY):
       *                  - root directory (e.g.: M:\measurement_directory\measurement01) in this case all hosts subdirectories will be iterated,
       *                  - host directory (e.g.: M:\measurement_directory\measurement01\CARPC01),
       *                  - file path, path to file from measurement (e.g.: M:\measurement_directory\measurement01\CARPC01\meas01_05.hdf5).
       *
       *                 File path as output (eAccessType::CREATE):
       *                  - full path to  measurement directory (recommended with host name) (e.g.: M:\measurement_directory\measurement01\CARPC01),
       *                  - to set the name of the actual hdf5 file use SetFileBaseName method.
       *
       * @param access   Access type
       *
       * @return         true if output (eAccessType::CREATE) measurement directory structure can be accessed/created, false otherwise.
       *                 true if input (eAccessType::RDONLY) measurement/file path was opened, false otherwise.
      **/
      bool Open(const std::string& path, eAccessType access = eAccessType::RDONLY);

      /**
       * @brief Close file
       *
       * @return         true if succeeds, false if it fails
      **/
      bool Close();

      /**
       * @brief Checks if file/measurement is ok
       *
       * @return  true if meas can be opened(read) or location is accessible(write), false otherwise
      **/
      bool IsOk() const;

      /**
       * @brief Get the File Type Version of the current opened file
       *
       * @return       file version
      **/
      std::string GetFileVersion() const;

      /**
       * @brief Gets maximum allowed size for an individual file
       *
       * @return       maximum size in MB
      **/
      size_t GetMaxSizePerFile() const;

      /**
       * @brief Sets maximum allowed size for an individual file
       *
       * @param size   maximum size in MB
      **/
      void SetMaxSizePerFile(size_t size);

      /**
       * @brief Get the available channel names of the current opened file / measurement
       *
       * @return       channel names
      **/
      std::set<std::string> GetChannelNames() const;

      /**
       * @brief Check if channel exists in measurement
       *
       * @param channel_name   name of the channel
       *
       * @return       true if exists, false otherwise
      **/
      bool HasChannel(const std::string& channel_name) const;

      /**
       * @brief Get the channel description for the given channel
       *
       * @param channel_name  channel name
       *
       * @return              channel description
      **/
      std::string GetChannelDescription(const std::string& channel_name) const;

      /**
       * @brief Set description of the given channel
       *
       * @param channel_name    channel name
       * @param description     description of the channel
      **/
      void SetChannelDescription(const std::string& channel_name, const std::string& description);

      /**
       * @brief Gets the channel type of the given channel
       *
       * @param channel_name  channel name
       *
       * @return              channel type
      **/
      std::string GetChannelType(const std::string& channel_name) const;

      /**
       * @brief Set type of the given channel
       *
       * @param channel_name  channel name
       * @param type          type of the channel
      **/
      void SetChannelType(const std::string& channel_name, const std::string& type);

      /**
       * @brief Gets minimum timestamp for specified channel
       *
       * @param channel_name    channel name
       *
       * @return                minimum timestamp value
      **/
      long long GetMinTimestamp(const std::string& channel_name) const;

      /**
       * @brief Gets maximum timestamp for specified channel
       *
       * @param channel_name    channel name
       *
       * @return                maximum timestamp value
      **/
      long long GetMaxTimestamp(const std::string& channel_name) const;

      /**
       * @brief Gets the header info for all data entries for the given channel
       *        Header = timestamp + entry id
       *
       * @param [in]  channel_name  channel name
       * @param [out] entries       header info for all data entries
       *
       * @return                    true if succeeds, false if it fails
      **/
      bool GetEntriesInfo(const std::string& channel_name, EntryInfoSet& entries) const;

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
      bool GetEntriesInfoRange(const std::string& channel_name, long long begin, long long end, EntryInfoSet& entries) const;

      /**
       * @brief Gets data size of a specific entry
       *
       * @param [in]  entry_id   Entry ID
       * @param [out] size       Entry data size
       *
       * @return                 true if succeeds, false if it fails
      **/
      bool GetEntryDataSize(long long entry_id, size_t& size) const;

      /**
       * @brief Gets data from a specific entry
       *
       * @param [in]  entry_id   Entry ID
       * @param [out] data       Entry data
       *
       * @return                 true if succeeds, false if it fails
      **/
      bool GetEntryData(long long entry_id, void* data) const;

      /**
       * @brief Set measurement file base name (desired name for the actual hdf5 files that will be created)
       *
       * @param base_name        Name of the hdf5 files that will be created.
      **/
      void SetFileBaseName(const std::string& base_name);

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
      bool AddEntryToFile(const void* data, const unsigned long long& size, const long long& snd_timestamp, const long long& rcv_timestamp, const std::string& channel_name, long long id, long long clock);

      /**
       * @brief Callback function type for pre file split notification
      **/
      typedef std::function<void(void)> CallbackFunction;

      /**
       * @brief Connect callback for pre file split notification
       *
       * @param cb   callback function
      **/
      void ConnectPreSplitCallback(CallbackFunction cb);

      /**
       * @brief Disconnect pre file split callback
      **/
      void DisconnectPreSplitCallback();

     private:
      HDF5MeasImpl* hdf_meas_impl_;
    };
  }  // namespace eh5
}  // namespace eCAL
