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
 * eCALHDF5 file reader single channel
**/

#pragma once

#include <list>
#include <map>
#include <string>
#include <unordered_map>

#include "eh5_meas_impl.h"

#include "hdf5.h"

namespace eCAL
{
  namespace eh5
  {
    class HDF5MeasFileWriterV5 : virtual public HDF5MeasImpl
    {
    public:
      /**
      * @brief Constructor
      **/
      HDF5MeasFileWriterV5();

      // Copy
      HDF5MeasFileWriterV5(const HDF5MeasFileWriterV5&)            = delete;
      HDF5MeasFileWriterV5& operator=(const HDF5MeasFileWriterV5&) = delete;

      // Move
      HDF5MeasFileWriterV5& operator=(HDF5MeasFileWriterV5&&)      = default;
      HDF5MeasFileWriterV5(HDF5MeasFileWriterV5&&)                 = default;

      /**
      * @brief Destructor
      **/
      ~HDF5MeasFileWriterV5() override;

      /**
      * @brief Open file
      *
      * @param output_dir  Input file path / measurement directory path
      * @param access      Access type (IGNORED, WILL ALWAYS OPEN READ-WRITE!)
      *
      * @return            true if succeeds, false if it fails
      **/
      bool Open(const std::string& output_dir, eAccessType access) override;

      /**
      * @brief Close file
      *
      * @return         true if succeeds, false if it fails
      **/
      bool Close() override;

      /**
      * @brief Checks if file/measurement is ok
      *
      * @return  true if meas can be opened(read) or location is accessible(write), false otherwise
      **/
      bool IsOk() const override;

      /**
      * @brief Get the File Type Version of the current opened file
      *
      * @return       file version
      **/
      std::string GetFileVersion() const override;

      /**
      * @brief Gets maximum allowed size for an individual file
      *
      * @return       maximum size in MB
      **/
      size_t GetMaxSizePerFile() const override;

      /**
      * @brief Sets maximum allowed size for an individual file
      *
      * @param max_file_size_mib   maximum size in MB
      **/
      void SetMaxSizePerFile(size_t max_file_size_mib) override;

      /**
      * @brief Whether each Channel shall be writte in its own file
      * 
      * When enabled, data is clustered by channel and each channel is written
      * to its own file. The filenames will consist of the basename and the 
      * channel name.
      * 
      * @return true, if one file per channel is enabled
      */
      bool IsOneFilePerChannelEnabled() const override;

      /**
      * @brief Enable / disable the creation of one individual file per channel
      * 
      * When enabled, data is clustered by channel and each channel is written
      * to its own file. The filenames will consist of the basename and the 
      * channel name.
      * 
      * @param enabled   Whether one file shall be created per channel
      */
      void SetOneFilePerChannelEnabled(bool enabled) override;

      /**
      * @brief Get the available channel names of the current opened file / measurement
      *
      * @return       channel names
      **/
      std::set<std::string> GetChannelNames() const override;

      /**
      * @brief Check if channel exists in measurement
      *
      * @param channel_name   name of the channel
      *
      * @return       true if exists, false otherwise
      **/
      bool HasChannel(const std::string& channel_name) const override;

      /**
      * @brief Get the channel description for the given channel
      *
      * @param channel_name  channel name
      *
      * @return              channel description
      **/
      std::string GetChannelDescription(const std::string& channel_name) const override;

      /**
      * @brief Set description of the given channel
      *
      * @param channel_name    channel name
      * @param description     description of the channel
      **/
      void SetChannelDescription(const std::string& channel_name, const std::string& description) override;

      /**
      * @brief Gets the channel type of the given channel
      *
      * @param channel_name  channel name
      *
      * @return              channel type
      **/
      std::string GetChannelType(const std::string& channel_name) const override;

      /**
      * @brief Set type of the given channel
      *
      * @param channel_name  channel name
      * @param type          type of the channel
      **/
      void SetChannelType(const std::string& channel_name, const std::string& type) override;

      /**
      * @brief Gets minimum timestamp for specified channel
      *
      * @param channel_name    channel name
      *
      * @return                minimum timestamp value
      **/
      long long GetMinTimestamp(const std::string& channel_name) const override;

      /**
      * @brief Gets maximum timestamp for specified channel
      *
      * @param channel_name    channel name
      *
      * @return                maximum timestamp value
      **/
      long long GetMaxTimestamp(const std::string& channel_name) const override;

      /**
      * @brief Gets the header info for all data entries for the given channel
      *        Header = timestamp + entry id
      *
      * @param [in]  channel_name  channel name
      * @param [out] entries       header info for all data entries
      *
      * @return                    true if succeeds, false if it fails
      **/
      bool GetEntriesInfo(const std::string& channel_name, EntryInfoSet& entries) const override;

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
      bool GetEntriesInfoRange(const std::string& channel_name, long long begin, long long end, EntryInfoSet& entries) const override;

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
      * @brief Set measurement file base name
      *
      * @param base_name        File base name.
      **/
      void SetFileBaseName(const std::string& base_name) override;

      /**
      * @brief Add entry to file
      *
      * @param data           data to be added
      * @param size           size of the data
      * @param snd_timestamp  send timestamp
      * @param rcv_timestamp  receive timestamp
      * @param channel_name   channel name
      * @param id             message id
      * @param clock          message clock
      *
      * @return               true if succeeds, false if it fails
      **/
      bool AddEntryToFile(const void* data, const unsigned long long& size, const long long& snd_timestamp, const long long& rcv_timestamp, const std::string& channel_name, long long id, long long clock) override;

      using CallbackFunction = std::function<void ()>;
      /**
      * @brief Connect callback for pre file split notification
      *
      * @param cb   callback function
      **/
      void ConnectPreSplitCallback(CallbackFunction cb) override;

      /**
      * @brief Disconnect pre file split callback
      **/
      void DisconnectPreSplitCallback() override;

    protected:
      struct Channel
      {
        std::string   Description;
        std::string   Type;
        EntryInfoVect Entries;
      };

      using Channels = std::map<std::string, Channel>;

      std::string              output_dir_;
      std::string              base_name_;
      Channels                 channels_;
      CallbackFunction         cb_pre_split_;
      hid_t                    file_id_;
      int                      file_split_counter_;
      unsigned long long       entries_counter_;
      size_t                   max_size_per_file_;

      /**
      * @brief Creates the actual file
      *
      * @return       file ID, file was not created if id is negative
      **/
      hid_t Create();

      /**
      * @brief Set attribute to object(file, entry...)
      *
      * @param id       ID of the attributes parent
      * @param name     Name of the attribute
      * @param value    Value of the attribute
      *
      * @return         true if succeeds, false if it fails
      **/
      static bool SetAttribute(const hid_t& id, const std::string& name, const std::string& value);

      /**
      * @brief Checks if current file size + entry size does not exceed the maximum allowed size of the file
      *
      * @param size  Size of the entry in bytes
      *
      * @return  true if entry can be saved in current file, false if it can not be added to the current file
      **/
      bool EntryFitsTheFile(const hsize_t& size) const;

      /**
      * @brief Gets the size of the file
      *
      * @param size  Size of the file in bytes
      *
      * @return  true if succeeds, false if it fails
      **/
      bool GetFileSize(hsize_t& size) const;

      /**
      * @brief Creates the entries "table of contents" (timestamp + entry id)
      *        (Call it just before closing the file)
      *
      * @param channelName         name for the dataset
      * @param channelType         type for the dataset
      * @param channelDescription  description for the dataset
      * @param entries             entries for given channel
      *
      * @return                    true if succeeds, false if it fails
      **/
      bool CreateEntriesTableOfContentsFor(const std::string& channelName, const std::string& channelType, const std::string& channelDescription, const EntryInfoVect& entries) const;

    };
  }  //  namespace eh5
}  //  namespace eCAL
