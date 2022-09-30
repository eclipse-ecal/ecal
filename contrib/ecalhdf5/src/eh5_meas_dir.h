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
#include <memory>

#include "eh5_meas_impl.h"

#include "hdf5.h"

#include "ecalhdf5/eh5_meas.h"

namespace eCAL
{
  namespace eh5
  {
    class HDF5MeasDir : virtual public HDF5MeasImpl
    {
    public:
      /**
      * @brief Constructor
      **/
      HDF5MeasDir();

      /**
      * @brief Constructor
      *
      * @param path    input file path
      **/
      explicit HDF5MeasDir(const std::string& path, eAccessType access = eAccessType::RDONLY);

      /**
      * @brief Destructor
      **/
      ~HDF5MeasDir() override;

      /**
      * @brief Open file
      *
      * @param path     Input file path / measurement directory path
      * @param access   Access type
      *
      * @return         true if succeeds, false if it fails
      **/
      bool Open(const std::string& path, eAccessType access = eAccessType::RDONLY) override;

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


    // =====================================================================
    // ==== Reading Files
    // =====================================================================
    protected:
      struct ChannelInfo
      {
        std::string type;
        std::string description;
        std::list<const eCAL::eh5::HDF5Meas*> files;

        ChannelInfo() = default;
        ChannelInfo(const std::string& type_, const std::string& description_)
          : type(type_)
          , description(description_)
        {}
      };

      struct EntryInfo
      {
        long long                   file_id;
        const eCAL::eh5::HDF5Meas* reader;

        EntryInfo() : file_id(0), reader(nullptr) {}

        EntryInfo(long long file_id_, const eCAL::eh5::HDF5Meas* reader_)
          : file_id(file_id_)
          , reader(reader_)
        {}
      };

      typedef std::list<eCAL::eh5::HDF5Meas*>               HDF5Files;
      typedef std::unordered_map<std::string, ChannelInfo>  ChannelInfoUMap;
      typedef std::unordered_map<long long, EntryInfo>      EntriesByIdUMap;
      typedef std::unordered_map<std::string, EntryInfoSet> EntriesByChannelUMap;

      HDF5Files              file_readers_;
      ChannelInfoUMap        channels_info_;
      EntriesByIdUMap        entries_by_id_;
      EntriesByChannelUMap   entries_by_chn_;

      struct Channel
      {
        std::string   Description;
        std::string   Type;
        EntryInfoVect Entries;
      };

      typedef std::map<std::string, Channel> Channels;

      Channels                 channels_;
      eAccessType              access_;

      std::list<std::string> GetHdfFiles(const std::string& path) const;

      static inline bool HasHdf5Extension(const std::string& str)
      {
        std::string end(".hdf5");
        if (end.size() > str.size()) return false;
        return std::equal(end.rbegin(), end.rend(), str.rbegin());
      }

      bool OpenRX(const std::string& path, eAccessType access /*= eAccessType::RDONLY*/);


      // =====================================================================
      // ==== Writing files
      // =====================================================================
    protected:
      typedef std::unordered_map<std::string, std::unique_ptr<::eCAL::eh5::HDF5MeasImpl>> FileWriterMap;

      std::string         output_dir_;                                          //!< The directory where the HDF5 files shall be placed when in CREATE mode
      std::string         base_name_;                                           //!< The filename of HDF5 files when in CREATE mode. Will be postfixed by the channel name when in one_file_per_channel_ mode. Will be further postfixed by a number when the files are splitted.
      bool                one_file_per_channel_;                                //!< If true, one FileWriter will be created for each channel.
      FileWriterMap       file_writers_;                                        //!< Map of {ChannelName -> FileWriter}. Grows for each new channel, if one_file_per_channel_ is true. Contains only one "" key otherwise that is used for all channels. 

      size_t              max_size_per_file_;                                   //!< Maximum file size after which the File Writer shall split
      CallbackFunction    cb_pre_split_;                                        //!< Callback that is executed before a new hdf5 file is created during splitting. Will be executed by each file writer individually.

    protected:
      /**
       * @brief Returns a writer for the given channel name
       * 
       * If one_file_per_channel_ is true, one writer for each channel will be
       * create and / or returned. Otherwise, only one writer will exist and
       * be reused for all channels.
       * 
       * New writers will be initialized with the split size and callback that
       * were set in this class.
       * 
       * @param channel_name  The channel name to return a writer for
       * 
       * @return an iterator to the writer
       */
      FileWriterMap::iterator GetWriter(const std::string& channel_name);
    };
  }  //  namespace eh5
}  //  namespace eCAL
