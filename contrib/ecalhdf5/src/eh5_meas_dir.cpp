/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
 * @brief  eCALHDF5 directory reader
**/

#include "eh5_meas_dir.h"
#include "escape.h"

#define NOMINMAX
#ifdef WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif //WIN32

#include <iostream>
#include <limits>
#include <list>
#include <string>

#include <ecal_utils/filesystem.h>
#include <ecal_utils/str_convert.h>

#include "eh5_meas_file_writer_v5.h"
#include "eh5_meas_file_writer_v6.h"

// TODO: Test the one-file-per-channel setting with gtest
constexpr unsigned int kDefaultMaxFileSizeMB = 1000;
eCAL::eh5::HDF5MeasDir::HDF5MeasDir()
  : access_              (v3::eAccessType::RDONLY) // Temporarily set it to RDONLY, so the leading "Close()" from the Open() function will not operate on the uninitialized variable.
  , one_file_per_channel_(false)
  , max_size_per_file_   (kDefaultMaxFileSizeMB * 1024 * 1024)
  , cb_pre_split_        (nullptr)
{}

eCAL::eh5::HDF5MeasDir::HDF5MeasDir(const std::string& path, v3::eAccessType access /*= eAccessType::RDONLY*/)
  : access_              (access)
  , one_file_per_channel_(false)
  , max_size_per_file_   (kDefaultMaxFileSizeMB * 1024 * 1024)
  , cb_pre_split_        (nullptr)
{
  // call the function via its class becase it's a virtual function that is called in constructor/destructor,-
  // where the vtable is not created yet or it's destructed.
  HDF5MeasDir::Open(path, access_);
}

eCAL::eh5::HDF5MeasDir::~HDF5MeasDir()
{
  // call the function via its class becase it's a virtual function that is called in constructor/destructor,-
  // where the vtable is not created yet or it's destructed.
  HDF5MeasDir::Close();
}

bool eCAL::eh5::HDF5MeasDir::Open(const std::string& path, v3::eAccessType access /*= eAccessType::RDONLY*/)
{
  // call the function via its class becase it's a virtual function that is called directly/indirectly in constructor/destructor,-
  // where the vtable is not created yet or it's destructed.
  HDF5MeasDir::Close();

  // Check if the given path points to a directory
  if (!EcalUtils::Filesystem::IsDir(path, EcalUtils::Filesystem::Current))
    return false;

  access_ = access;

  switch (access)
  {
  case eCAL::eh5::v3::eAccessType::RDONLY:
  //case eCAL::eh5::RDWR:
    return OpenRX(path, access);
  case eCAL::eh5::v3::eAccessType::CREATE:
  case eCAL::eh5::v3::eAccessType::CREATE_V5:
    output_dir_ = path;
    return true;
  default:
    break;
  }

  return false;
}

bool eCAL::eh5::HDF5MeasDir::Close()
{
  bool successfully_closed{ true };

  if (access_ == v3::eAccessType::CREATE || access_ == v3::eAccessType::CREATE_V5)
  {
    // Close all existing file writers
    for (auto& file_writer : file_writers_)
    {
      successfully_closed &= file_writer.second->Close();
    }

    // Clear the list of all file writers, which will delete them
    file_writers_.clear();

    return successfully_closed;
  }
  else
  {
    for (auto file : file_readers_)
    {
      if (file != nullptr)
      {
        successfully_closed &= file->Close();
        delete file;
        file = nullptr;
      }
    }

    file_readers_.clear();
    channels_info_.clear();
    entries_by_id_.clear();
    entries_by_chn_.clear();

    return successfully_closed;
  }
}

bool eCAL::eh5::HDF5MeasDir::IsOk() const
{
  switch (access_)
  {
  case eCAL::eh5::v3::eAccessType::RDONLY:
  //case eCAL::eh5::RDWR:
    return !file_readers_.empty() && !entries_by_id_.empty();
  case eCAL::eh5::v3::eAccessType::CREATE:
  case eCAL::eh5::v3::eAccessType::CREATE_V5:
    return true;
  default:
    return false;
  }
}

std::string eCAL::eh5::HDF5MeasDir::GetFileVersion() const
{
  std::string version;
  if (!file_readers_.empty())
  {
    version = file_readers_.front()->GetFileVersion();
  }
  return version;
}

size_t eCAL::eh5::HDF5MeasDir::GetMaxSizePerFile() const
{
  return max_size_per_file_ / 1024 / 1024;
}

void eCAL::eh5::HDF5MeasDir::SetMaxSizePerFile(size_t max_file_size_mib)
{
  // Store to internal variable
  max_size_per_file_ = max_file_size_mib * 1024 * 1024;

  // Update all file writers (if there are any)
  for (auto& file_writer : file_writers_)
  {
    file_writer.second->SetMaxSizePerFile(max_file_size_mib);
  }
}

bool eCAL::eh5::HDF5MeasDir::IsOneFilePerChannelEnabled() const
{
  return one_file_per_channel_;
}

void eCAL::eh5::HDF5MeasDir::SetOneFilePerChannelEnabled(bool enabled)
{
  one_file_per_channel_ = enabled;
}

std::set<eCAL::eh5::SChannel> eCAL::eh5::HDF5MeasDir::GetChannels() const
{
  std::set<eCAL::eh5::SChannel> channels;

  for (const auto& chn : channels_info_)
    channels.insert(chn.first);

  return channels;
}

bool eCAL::eh5::HDF5MeasDir::HasChannel(const eCAL::eh5::SChannel& channel) const
{
  return channels_info_.find(channel) != channels_info_.end();
}

eCAL::eh5::DataTypeInformation eCAL::eh5::HDF5MeasDir::GetChannelDataTypeInformation(const SChannel& channel) const
{
  eCAL::eh5::DataTypeInformation ret_val;

  const auto& found = channels_info_.find(channel);
  if (found != channels_info_.end())
  {
    ret_val = found->second.info;
  }
  return ret_val;
}

// TODO: this seems fishy. Do we need to escape?
void eCAL::eh5::HDF5MeasDir::SetChannelDataTypeInformation(const SChannel& channel, const eCAL::eh5::DataTypeInformation& info)
{
  // Get an existing writer or create a new one
  auto file_writer_it = GetWriter(channel);
  file_writer_it->second->SetChannelDataTypeInformation(channel, info);

  // Let's save them in case we need to query them
  channels_info_[channel] = ChannelInfo(info);
}

long long eCAL::eh5::HDF5MeasDir::GetMinTimestamp(const SChannel& channel) const
{
  long long min_timestamp = std::numeric_limits<long long>::max();
  const auto& channel_entries = entries_by_chn_.find(channel);

  if (channel_entries != entries_by_chn_.end())
  {
    if (!channel_entries->second.empty())
    {
      min_timestamp = channel_entries->second.begin()->RcvTimestamp;
    }
  }

  return min_timestamp;
}

long long eCAL::eh5::HDF5MeasDir::GetMaxTimestamp(const SChannel& channel) const
{
  long long max_timestamp = std::numeric_limits<long long>::min();
  const auto& channel_entries = entries_by_chn_.find(channel);

  if (channel_entries != entries_by_chn_.end())
  {
    if (!channel_entries->second.empty())
    {
      max_timestamp = channel_entries->second.rbegin()->RcvTimestamp;
    }
  }

  return max_timestamp;
}

bool eCAL::eh5::HDF5MeasDir::GetEntriesInfo(const SChannel& channel, EntryInfoSet& entries) const
{
  entries.clear();

  const auto& channel_it = entries_by_chn_.find(channel);
  if (channel_it == entries_by_chn_.end())
  {
    return false;
  }

  entries = channel_it->second;

  return !entries.empty();
}

bool eCAL::eh5::HDF5MeasDir::GetEntriesInfoRange(const SChannel& channel, long long begin, long long end, EntryInfoSet& entries) const
{
  entries.clear();

  const auto& channel_it = entries_by_chn_.find(channel);
  if (channel_it == entries_by_chn_.end())
  {
    return false;
  }

  if (begin == 0) begin = entries.begin()->RcvTimestamp;
  if (end == 0) end = entries.rbegin()->RcvTimestamp;

  const auto& lower = channel_it->second.lower_bound(SEntryInfo(begin, 0, 0));
  const auto& upper = channel_it->second.upper_bound(SEntryInfo(end, 0, 0));

  entries.insert(lower, upper);
  return true;
}

bool eCAL::eh5::HDF5MeasDir::GetEntryDataSize(long long entry_id, size_t& size) const
{
  auto ret_val = false;
  const auto& found = entries_by_id_.find(entry_id);
  if (found != entries_by_id_.end())
  {
    ret_val = found->second.reader->GetEntryDataSize(found->second.file_id, size);
  }
  return ret_val;
}

bool eCAL::eh5::HDF5MeasDir::GetEntryData(long long entry_id, void* data) const
{
  auto ret_val = false;
  const auto& found = entries_by_id_.find(entry_id);
  if (found != entries_by_id_.end())
  {
    ret_val = found->second.reader->GetEntryData(found->second.file_id, data);
  }
  return ret_val;
}

void eCAL::eh5::HDF5MeasDir::SetFileBaseName(const std::string& base_name)
{
  base_name_ = base_name;
}

bool eCAL::eh5::HDF5MeasDir::AddEntryToFile(const SWriteEntry& entry)
{
  if ((access_ == v3::eAccessType::RDONLY)
    || (output_dir_.empty())
    || (base_name_.empty()))
  {
    return false;
  }

  // Get an existing writer or create a new one
  auto file_writer_it = GetWriter(entry.channel);

  // Use the writer that was either found or created to actually write the data
  return file_writer_it->second->AddEntryToFile(entry);
}

void eCAL::eh5::HDF5MeasDir::ConnectPreSplitCallback(CallbackFunction cb)
{
  // Store the callback function internally
  cb_pre_split_ = cb;

  // If there are any existing file_writers, add the Callback to all of them.
  // They will effectively call the callback, as they are handling the splitting.
  for (const auto& file_writer : file_writers_)
  {
    file_writer.second->ConnectPreSplitCallback(cb_pre_split_);
  }
}

void eCAL::eh5::HDF5MeasDir::DisconnectPreSplitCallback()
{
  // Clear internal copy of the callback
  cb_pre_split_ = nullptr;

  // If there are any existing file_writers, remove the Callback from all of them.
  for (const auto& file_writer : file_writers_)
  {
    file_writer.second->DisconnectPreSplitCallback();
  }
}

std::list<std::string> eCAL::eh5::HDF5MeasDir::GetHdfFiles(const std::string& path) const
{
  std::list<std::string> paths;
#ifdef WIN32
  std::string dpath = path + "/*.*";
  std::wstring dpath_w = EcalUtils::StrConvert::Utf8ToWide(dpath);

  WIN32_FIND_DATAW fd;
  HANDLE hFind = ::FindFirstFileW(dpath_w.c_str(), &fd);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    do {
      std::wstring file_name_w(fd.cFileName);
      std::string file_name_utf8 = EcalUtils::StrConvert::WideToUtf8(file_name_w);

      if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
      {
        if (HasHdf5Extension(file_name_utf8) == true)
        {
          paths.push_back(path + "/" + std::string(file_name_utf8.begin(), file_name_utf8.end()));
        }
      }
      else
      {
        if (file_name_utf8 != "." && file_name_utf8 != "..")
        {
          paths.splice(paths.end(), GetHdfFiles(path + "/" + std::string(file_name_utf8.begin(), file_name_utf8.end())));
        }
      }
    } while (::FindNextFileW(hFind, &fd));
    ::FindClose(hFind);
  }
#else
  struct dirent* de = nullptr;
  DIR* dir = nullptr;

  dir = opendir(path.c_str());

  if (dir != nullptr)
  {
    de = readdir(dir);
    while (de != nullptr)
    {
      std::string d_name = de->d_name;
      if (d_name != "." && d_name != "..")
      {
        if (de->d_type == DT_DIR)
        {
          paths.splice(paths.end(), GetHdfFiles(path + "/" + d_name));
        }
        else
        {
          if (HasHdf5Extension(d_name))
            paths.push_back(path + "/" + d_name);
        }
      }
      de = readdir(dir);
    }
    closedir(dir);
}
#endif  //  WIN32
  return paths;
}

bool eCAL::eh5::HDF5MeasDir::OpenRX(const std::string& path, v3::eAccessType access /*= eAccessType::RDONLY*/)
{
  if (access != v3::eAccessType::RDONLY /*&& access != eAccessType::RDWR*/) return false;

  auto files = GetHdfFiles(path);

  long long id = 0;

  for (const auto& file_path : files)
  {
    auto reader = new eCAL::eh5::v3::HDF5Meas(file_path);

    if (reader->IsOk())
    {
      auto channels = reader->GetChannels();
      for (const auto& channel : channels)
      {
        auto escaped_channel = GetEscapedTopicname(channel);
        auto info = reader->GetChannelDataTypeInformation(escaped_channel);

        auto& channel_info = channels_info_[escaped_channel];
        channel_info.info = info;
        channel_info.files.push_back(reader);

        EntryInfoSet entries;
        if (reader->GetEntriesInfo(channel, entries))
        {
          for (auto entry : entries)
          {
            entries_by_id_[id] = EntryInfo(entry.ID, reader);
            entry.ID = id;
            entries_by_chn_[escaped_channel].insert(entry);
            id++;
          }
        }
      }
      file_readers_.push_back(reader);
    }
    else
    {
      reader->Close();
      delete reader;
      reader = nullptr;
    }
  }
  return !file_readers_.empty();
}

::eCAL::eh5::HDF5MeasDir::FileWriterMap::iterator eCAL::eh5::HDF5MeasDir::GetWriter(const SChannel& channel)
{
  const auto& channel_name{ channel.name };
  // Look for an existing writer. When creating 1 file per channel, the channel
  // name is used as key. Otherwise, emptystring is used as "generic" key and
  // the same writer is used for all channels.
  FileWriterMap::iterator file_writer_it = file_writers_.find(one_file_per_channel_ ? channel_name : "");
  if (file_writer_it == file_writers_.end())
  {
    if (access_ == v3::eAccessType::CREATE)
    {
      // No appropriate file writer was found. Let's create a new one!
      file_writer_it = file_writers_.emplace(one_file_per_channel_ ? channel_name : "", std::make_unique<::eCAL::eh5::HDF5MeasFileWriterV6>()).first;
    }
    else
    {
      file_writer_it = file_writers_.emplace(one_file_per_channel_ ? channel_name : "", std::make_unique<::eCAL::eh5::HDF5MeasFileWriterV5>()).first;
    }

    // Set the current parameters to the new file writer
    file_writer_it->second->SetMaxSizePerFile(GetMaxSizePerFile());
    file_writer_it->second->SetOneFilePerChannelEnabled(one_file_per_channel_);
    file_writer_it->second->SetFileBaseName(one_file_per_channel_ ? (base_name_ + "_" + GetEscapedFilename(GetUnescapedString(channel_name))) : (base_name_));
    if (cb_pre_split_)
      file_writer_it->second->ConnectPreSplitCallback(cb_pre_split_);

    // Open the writer
    file_writer_it->second->Open(output_dir_);
  }

  // The iterator is either what we found or what we created. In either way it
  // will be valid and can be returned.
  return file_writer_it;
}
