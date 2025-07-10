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
 * @brief  eCALHDF5 measurement file version V2
**/

#pragma once

#include "hdf5.h"
#include "eh5_meas_impl.h"
#include "escape.h"

namespace eCAL
{
  namespace eh5
  {
    class HDF5MeasFileV2 : virtual public HDF5MeasImpl
    {
    public:
      /**
      * @brief Constructor
      **/
      HDF5MeasFileV2();

      /**
      * @brief Constructor
      *
      * @param path    input file path
      **/
      explicit HDF5MeasFileV2(const std::string& path, v3::eAccessType access = v3::eAccessType::RDONLY);

      /**
      * @brief Destructor
      **/
      ~HDF5MeasFileV2() override;

      /**
      * @brief Open file
      *
      * @param path     Input file path / measurement directory path
      * @param access   Access type
      *
      * @return         true if succeeds, false if it fails
      **/
      bool Open(const std::string& path, v3::eAccessType access = v3::eAccessType::RDONLY) override;

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
      * @param size   maximum size in MB
      **/
      void SetMaxSizePerFile(size_t size) override;

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
       * @return       channel names & ids
      **/
      std::set<eCAL::eh5::SEscapedChannel> GetChannels() const override;

      /**
       * @brief Check if channel exists in measurement
       *
       * @param channel   channel name & id
       *
       * @return       true if exists, false otherwise
      **/
      bool HasChannel(const eCAL::eh5::SEscapedChannel& channel) const override;

      /**
       * @brief Get data type information of the given channel
       *
       * @param channel_name  channel name
       *
       * @return              channel type
      **/
      DataTypeInformation GetChannelDataTypeInformation(const SEscapedChannel& channel) const override;

      /**
       * @brief Set data type information of the given channel
       *
       * @param channel_name  channel name
       * @param info          datatype info of the channel
       *
       * @return              channel type
      **/
      void SetChannelDataTypeInformation(const SEscapedChannel& channel, const DataTypeInformation& info) override;

      /**
      * @brief Gets minimum timestamp for specified channel
      *
      * @param channel_name    channel name
      *
      * @return                minimum timestamp value
      **/
      long long GetMinTimestamp(const SEscapedChannel& channel) const override;

      /**
      * @brief Gets maximum timestamp for specified channel
      *
      * @param channel_name    channel name
      *
      * @return                maximum timestamp value
      **/
      long long GetMaxTimestamp(const SEscapedChannel& channel) const override;

      /**
      * @brief Gets the header info for all data entries for the given channel
      *        Header = timestamp + entry id
      *
      * @param [in]  channel_name  channel name
      * @param [out] entries       header info for all data entries
      *
      * @return                    true if succeeds, false if it fails
      **/
      bool GetEntriesInfo(const SEscapedChannel& channel, EntryInfoSet& entries) const override;

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
      bool GetEntriesInfoRange(const SEscapedChannel& channel, long long begin, long long end, EntryInfoSet& entries) const override;

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
      bool AddEntryToFile(const SEscapedWriteEntry& entry) override;


      typedef std::function<void(void)> CallbackFunction;
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
      hid_t file_id_;
    };

  }  // namespace eh5
}  // namespace eCAL
