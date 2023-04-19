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
 * @file   measurement.h
 * @brief  Base class for low level measurement oprations
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
     * @brief eCAL measurement API
    **/
    class Measurement
    {
    public:
      /**
       * @brief Constructor
      **/
      Measurement() = default;

      /**
       * @brief Destructor
      **/
      virtual ~Measurement() = default;

      /**
       * @brief Copy operator
      **/
      Measurement(const Measurement& other) = delete;
      Measurement& operator=(const Measurement& other) = delete;

      /**
      * @brief Move operator
      **/
      Measurement(Measurement&&) = default;
      Measurement& operator=(Measurement&&) = default;

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
       *                 File path as input (AccessType::RDONLY):
       *                  - root directory (e.g.: M:\measurement_directory\measurement01) in this case all hosts subdirectories will be iterated,
       *                  - host directory (e.g.: M:\measurement_directory\measurement01\CARPC01),
       *                  - file path, path to file from measurement (e.g.: M:\measurement_directory\measurement01\CARPC01\meas01_05.hdf5).
       *
       *                 File path as output (AccessType::CREATE):
       *                  - full path to  measurement directory (recommended with host name) (e.g.: M:\measurement_directory\measurement01\CARPC01),
       *                  - to set the name of the actual hdf5 file use SetFileBaseName method.
       *
       * @param access   Access type
       *
       * @return         true if output (AccessType::CREATE) measurement directory structure can be accessed/created, false otherwise.
       *                 true if input (AccessType::RDONLY) measurement/file path was opened, false otherwise.
      **/
      virtual bool Open(const std::string& path, AccessType access = AccessType::RDONLY)  = 0;

      /**
       * @brief Close file
       *
       * @return         true if succeeds, false if it fails
      **/
      virtual bool Close()  = 0;

      /**
       * @brief Checks if file/measurement is ok
       *
       * @return  true if meas can be opened(read) or location is accessible(write), false otherwise
      **/
      virtual bool IsOk() const  = 0;

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
      virtual size_t GetMaxSizePerFile() const = 0;

      /**
       * @brief Sets maximum allowed size for an individual file
       *
       * @param size   maximum size in MB
      **/
      virtual void SetMaxSizePerFile(size_t size) = 0;

      /**
      * @brief Whether each Channel shall be writte in its own file
      * 
      * When enabled, data is clustered by channel and each channel is written
      * to its own file. The filenames will consist of the basename and the 
      * channel name.
      * 
      * @return true, if one file per channel is enabled
      */
      virtual bool IsOneFilePerChannelEnabled() const = 0;

      /**
      * @brief Enable / disable the creation of one individual file per channel
      * 
      * When enabled, data is clustered by channel and each channel is written
      * to its own file. The filenames will consist of the basename and the 
      * channel name.
      * 
      * @param enabled   Whether one file shall be created per channel
      */
      virtual void SetOneFilePerChannelEnabled(bool enabled) = 0;

      /**
       * @brief Get the available channel names of the current opened file / measurement
       *
       * @return       channel names
      **/
      virtual std::set<std::string> GetChannelNames() const = 0;

      /**
       * @brief Check if channel exists in measurement
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
       * @brief Set description of the given channel
       *
       * @param channel_name    channel name
       * @param description     description of the channel
      **/
      virtual void SetChannelDescription(const std::string& channel_name, const std::string& description) = 0;

      /**
       * @brief Gets the channel type of the given channel
       *
       * @param channel_name  channel name
       *
       * @return              channel type
      **/
      virtual std::string GetChannelType(const std::string& channel_name) const = 0;

      /**
       * @brief Set type of the given channel
       *
       * @param channel_name  channel name
       * @param type          type of the channel
      **/
      virtual void SetChannelType(const std::string& channel_name, const std::string& type) = 0;

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

      /**
       * @brief Set measurement file base name (desired name for the actual hdf5 files that will be created)
       *
       * @param base_name        Name of the hdf5 files that will be created.
      **/
      virtual void SetFileBaseName(const std::string& base_name) = 0;

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
      virtual bool AddEntryToFile(const void* data, const unsigned long long& size, const long long& snd_timestamp, const long long& rcv_timestamp, const std::string& channel_name, long long id, long long clock) = 0;

      /**
       * @brief Callback function type for pre file split notification
      **/
      typedef std::function<void(void)> CallbackFunction;

      /**
       * @brief Connect callback for pre file split notification
       *
       * @param cb   callback function
      **/
      virtual void ConnectPreSplitCallback(CallbackFunction cb) = 0;

      /**
       * @brief Disconnect pre file split callback
      **/
      virtual void DisconnectPreSplitCallback() = 0;
    };
    }
  }  // namespace eh5
}  // namespace eCAL
