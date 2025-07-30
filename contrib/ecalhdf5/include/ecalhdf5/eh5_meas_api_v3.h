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
 * @file   eh5_meas.h
 * @brief  eCALHDF5 measurement class
**/

#pragma once

#include <cstdint>
#include <functional>
#include <set>
#include <string>
#include <memory>

#include "eh5_types.h"


namespace eCAL
{
  namespace eh5
  {
    class HDF5MeasImpl;

    inline namespace v3{
      
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
      explicit HDF5Meas(const std::string& path, v3::eAccessType access = v3::eAccessType::RDONLY);

      /**
       * @brief Destructor
      **/
      ~HDF5Meas();

      /**
       * @brief Copy constructor deleted
      **/
      HDF5Meas(const HDF5Meas& other) = delete;
      /**
       * @brief Move assignemnt deleted
      **/
      HDF5Meas& operator=(const HDF5Meas& other) = delete;

      /**
      * @brief Move constructor
      **/
      HDF5Meas(HDF5Meas&&) = default;
      /**
       * @brief Move assignment
      **/
      HDF5Meas& operator=(HDF5Meas&&) = default;

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
      bool Open(const std::string& path, v3::eAccessType access = v3::eAccessType::RDONLY);

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
      * @brief Whether each Channel shall be writte in its own file
      * 
      * When enabled, data is clustered by channel and each channel is written
      * to its own file. The filenames will consist of the basename and the 
      * channel name.
      * 
      * @return true, if one file per channel is enabled
      */
      bool IsOneFilePerChannelEnabled() const;

      /**
      * @brief Enable / disable the creation of one individual file per channel
      * 
      * When enabled, data is clustered by channel and each channel is written
      * to its own file. The filenames will consist of the basename and the 
      * channel name.
      * 
      * @param enabled   Whether one file shall be created per channel
      */
      void SetOneFilePerChannelEnabled(bool enabled);

      /**
       * @brief Get the available channel names of the current opened file / measurement
       *
       * @return Channels (channel name & id)
      **/
      std::set<SChannel> GetChannels() const;

      /**
       * @brief Check if channel exists in measurement
       *
       * @param channel   channel name & id
       *
       * @return       true if exists, false otherwise
      **/
      bool HasChannel(const eCAL::eh5::SChannel& channel) const;

      /**
       * @brief Get data type information of the given channel
       *
       * @param channel_name  channel name
       *
       * @return              channel type
      **/
      DataTypeInformation GetChannelDataTypeInformation(const SChannel& channel) const;

      /**
       * @brief Set data type information of the given channel
       *
       * @param channel_name  channel name
       * @param info          datatype info of the channel
       *
       * @return              channel type
      **/
      void SetChannelDataTypeInformation(const SChannel& channel, const DataTypeInformation& info);

      /**
        * @brief Gets minimum timestamp for specified channel
        *
        * @param channel         channel (name & id)
        *
        * @return                minimum timestamp value
      **/
      long long GetMinTimestamp(const SChannel& channel) const;

      /**
       * @brief Gets maximum timestamp for specified channel
       *
       * @param channel         channel (name & id)
       *
       * @return                maximum timestamp value
      **/
      long long GetMaxTimestamp(const SChannel& channel) const;
     
      /**
       * @brief Gets the header info for all data entries for the given channel
       *        Header = timestamp + entry id
       *
       * @param [in]  channel       channel (name & id)
       * @param [out] entries       header info for all data entries
       *
       * @return                    true if succeeds, false if it fails
      **/
      bool GetEntriesInfo(const SChannel& channel, EntryInfoSet& entries) const;

      /**
       * @brief Gets the header info for data entries for the given channel included in given time range (begin->end)
       *        Header = timestamp + entry id
       *
       * @param [in]  channel      channel (name & id)
       * @param [in]  begin        time range begin timestamp
       * @param [in]  end          time range end timestamp
       * @param [out] entries      header info for data entries in given range
       *
       * @return                   true if succeeds, false if it fails
      **/
      bool GetEntriesInfoRange(const SChannel& channel, long long begin, long long end, EntryInfoSet& entries) const;

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
       * @param entry         the entry to be added to the file
       *
       * @return              true if succeeds, false if it fails
      **/
      bool AddEntryToFile(const SWriteEntry& entry);

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
      std::unique_ptr<HDF5MeasImpl> hdf_meas_impl_;
    };
    }
  }  // namespace eh5
}  // namespace eCAL
