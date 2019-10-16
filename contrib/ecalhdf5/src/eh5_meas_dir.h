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
      ~HDF5MeasDir();

      /**
      * @brief Open file
      *
      * @param path     Input file path / measurement directory path
      * @param access   Access type
      *
      * @return         true if succeeds, false if it fails
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
      * @brief Set measurement file base name
      *
      * @param base_name        File base name.
      **/
      void SetFileBaseName(const std::string& base_name);

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
      bool AddEntryToFile(const void* data, const unsigned long long& size, const long long& snd_timestamp, const long long& rcv_timestamp, const std::string& channel_name, long long id, long long clock);

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

    protected:
      struct ChannelInfo
      {
        std::string type;
        std::string description;
        std::list<const eCAL::eh5::HDF5Meas*> files;

        ChannelInfo() {}
        ChannelInfo(const std::string& type_, const std::string& description_)
          : type(type_)
          , description(description_)
        {}
      };

      struct EntryInfo
      {
        long long                     file_id;
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

      HDF5Files              files_;
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

      std::string              output_dir_;
      std::string              file_name_;
      Channels                 channels_;
      CallbackFunction         cb_pre_split_;
      hid_t                    file_id_;
      int                      file_split_counter_;
      unsigned long long       entries_counter_;
      size_t                   max_size_per_file_;
      eAccessType              access_;

      std::list<std::string> GetHdfFiles(const std::string& path) const;

      static inline bool HasHdf5Extension(const std::string& str)
      {
        std::string end(".hdf5");
        if (end.size() > str.size()) return false;
        return std::equal(end.rbegin(), end.rend(), str.rbegin());
      }

      /**
      * @brief Creates the actual file
      *
      * @return       file ID, file was not created if id is negative
      **/
      hid_t Create();

      bool OpenRX(const std::string& path, eAccessType access /*= eAccessType::RDONLY*/);

      /**
      * @brief Set attribute to object(file, entry...)
      *
      * @param id       ID of the attributes parent
      * @param name     Name of the attribute
      * @param value    Value of the attribute
      *
      * @return         true if succeeds, false if it fails
      **/
      bool SetAttribute(const hid_t& id, const std::string& name, const std::string& value);

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
      bool CreateEntriesTableOfContentsFor(const std::string& channelName, const std::string& channelType, const std::string& channelDescription, const EntryInfoVect& entries);

    };
  }  //  namespace eh5
}  //  namespace eCAL
